#include "lib/Analysis/ReduceNoiseAnalysis/ReduceNoiseAnalysis.h"

#include <string>

#include "lib/Dialect/Noisy/NoisyOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"
#include "ortools/linear_solver/linear_solver.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace operations_research;

namespace mlir {
namespace tutorial {

#define DEBUG_TYPE "ReduceNoiseAnalysis"

// This needs only be larger than 32, since we're hard coding i32s in this
// tutorial.
constexpr int IF_THEN_AUX = 100;

std::string nameAndLoc(Operation *op) {
  std::string varName;
  llvm::raw_string_ostream ss(varName);
  ss << op->getName() << "_" << op->getLoc();
  return ss.str();
}

// ============================================================================
// This function solves a Mixed-Integer Programming (MIP) problem using Google
// OR-Tools to decide WHERE to insert `reduce_noise` operations in a program.
//
// The core OR-Tools APIs work as follows:
//   - MPSolver    : The LP/MIP solver engine (here using "SCIP").
//   - MPVariable  : A variable in the optimization problem.
//   - MPObjective : The objective function to minimize or maximize.
//   - MPConstraint: A linear inequality/equality of the form:
//                      lower_bound <= Σ(coeff_i * var_i) <= upper_bound
//     Created via solver->MakeRowConstraint(lb, ub, name).
//   - SetCoefficient(var, coeff) : Adds the term `coeff * var` to a
//     constraint (or objective). So if you call:
//       constraint->SetCoefficient(x, 2);
//       constraint->SetCoefficient(y, -3);
//     the constraint represents:  2*x + (-3)*y  in the row.
//
// PROBLEM STATEMENT:
//   Each arithmetic op (Add/Sub/Mul) combines the noise from its operands and
//   produces a result with some noise level.  We can optionally insert a
//   `reduce_noise` op after each arithmetic op to reset the noise back to
//   INITIAL_NOISE (=12).  Inserting reduce_noise is costly (1 unit per op),
//   so we want as few as possible.  But we must also keep the noise at every
//   SSA value ≤ MAX_NOISE (=26).
//
// We model this as a MIP with two kinds of variables:
//
//   (A) Decision variables  d_i ∈ {0,1}  (binary, "IntVar(0,1)")
//       1 = insert reduce_noise after op i, 0 = don't insert.
//       Minimize:  Σ d_i   (minimize number of reduce_noise ops)
//
//   (B) Noise variables  n_v ∈ [0, MAX_NOISE]  (continuous, "NumVar")
//       The noise level carried by each SSA value v.
//
// Constraints link them together using a "big-M" technique (IF_THEN_AUX=100
// acts as the large constant).  The key idea: if d_i = 0 (no reduce_noise),
// then result_noise = input_noise.  If d_i = 1 (insert reduce_noise), then
// result_noise = INITIAL_NOISE (=12).  The big-M trick linearizes this
// conditional relationship without multiplying two variables together.
//
// NOTE: Variable bounds vs. Constraints
//   Every MPVariable already has a built-in range [lb, ub] set at creation
//   (e.g. IntVar(0,1) means 0 ≤ d_i ≤ 1 as part of the variable itself).
//   MPConstraints (MakeRowConstraint) are ADDITIONAL relationships that
//   involve MULTIPLE variables, like:  result_noise - 12*d ≥ 0.
//   Think of it as:
//     Variable bounds = "this variable lives in this box"     (implicit)
//     Constraints     = "these variables relate in this way"  (explicit)
//   Both are necessary — bounds keep variables finite, constraints model
//   the actual relationships between them.
// ============================================================================

ReduceNoiseAnalysis::ReduceNoiseAnalysis(Operation *op) {
  // --- Step 1: Create the solver and objective ---
  // MPSolver::CreateSolver("SCIP") creates a new MIP solver instance using
  // the SCIP backend.  SCIP is a free non-commercial solver for mixed-integer
  // programming.
  std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));

  // MPObjective is the objective function — the quantity we want to minimize
  // or maximize.  MutableObjective() returns a pointer to it.
  MPObjective *const objective = solver->MutableObjective();
  objective->SetMinimization();  // We want to MINIMIZE the number of reduce_noise ops.

  // decisionVariables[op] -> binary variable d_i ∈ {0,1} indicating whether
  //                          to insert reduce_noise after this op.
  //   Example: for IR like:
  //     %2 = noisy.add %0, %1     // call this op_A
  //     %3 = noisy.mul %2, %2     // call this op_B
  //   Then:
  //     decisionVariables[op_A] = d_A  (0 or 1)
  //     decisionVariables[op_B] = d_B  (0 or 1)
  //   And the objective is:  minimize  d_A + d_B
  llvm::DenseMap<Operation *, MPVariable *> decisionVariables;

  // ssaNoiseVariables[SSA value] -> continuous variable n_v ∈ [0, MAX_NOISE]
  //                                 tracking the noise level at each SSA value.
  //   Example: for the same IR above:
  //     ssaNoiseVariables[%0] = n_0  (input, fixed to 12 by Step 3)
  //     ssaNoiseVariables[%1] = n_1  (input, fixed to 12 by Step 3)
  //     ssaNoiseVariables[%2] = n_2  (result of add, constrained by big-M)
  //     ssaNoiseVariables[%3] = n_3  (result of mul, constrained by big-M)
  llvm::DenseMap<Value, MPVariable *> ssaNoiseVariables;

  // allVariables: keeps track of every variable for potential debugging output.
  std::vector<MPVariable *> allVariables;

  // --- Step 2: Walk the IR and create one variable per operation/value ---
  // For each arithmetic op, we create:
  //   - 1 binary decision variable  d_i  (whether to insert reduce_noise)
  //   - N continuous noise variables for its operands and result (if not
  //     already created by another op that shares the same SSA values).
  //
  // Concrete example for this MLIR snippet:
  //     %0 = noisy.encode ...
  //     %1 = noisy.encode ...
  //     %2 = noisy.add %0, %1      // op_A
  //     %3 = noisy.mul %2, %2      // op_B
  //
  // After Step 2, the solver's workspace looks like this:
  //   Variables:  d_A, d_B, n_0, n_1, n_2, n_3
  //   Objective:  minimize  1*d_A + 1*d_B
  //   (Constraints will be added in Step 3 & 4)
  op->walk([&](Operation *op) {
    // FIXME: assumes all reduce_noise ops have already been removed and their
    // values forwarded.
    if (!isa<noisy::AddOp, noisy::SubOp, noisy::MulOp>(op)) {
      return;
    }

    // --- Create the binary decision variable for this op ---
    // MakeIntVar(lower, upper, name) creates an INTEGER variable in [0, 1],
    // i.e. a binary variable.  1 = we will insert reduce_noise after this op.
    // In the objective, we give it coefficient 1, so minimizing the objective
    // means minimizing the number of ops where d_i = 1.
    std::string varName = "InsertReduceNoise_" + nameAndLoc(op);
    auto decisionVar = solver->MakeIntVar(0, 1, varName);
    decisionVariables.insert(std::make_pair(op, decisionVar));
    allVariables.push_back(decisionVar);
    //objective: minimize sum(d_i)=> so each di has coefficient 1 in the objective function.  The solver will try to set
    objective->SetCoefficient(decisionVar, 1);  // Each reduce_noise costs 1.

    // --- Create continuous noise variables for each operand ---
    // MakeNumVar(lower, upper, name) creates a CONTINUOUS (floating-point)
    // variable.  Here, the noise at each SSA value ranges from 0 to MAX_NOISE.
    // We only create a new variable if one doesn't already exist (because
    // multiple ops may share the same SSA operand value, e.g. a->b, a->c).
    int index = 0;
    for (auto operand : op->getOperands()) {
      if (ssaNoiseVariables.contains(operand)) {
        continue;
      }
      std::string varName =
          "NoiseAt_" + nameAndLoc(op) + "_arg_" + std::to_string(index++);
      auto ssaNoiseVar = solver->MakeNumVar(0, MAX_NOISE, varName);
      allVariables.push_back(ssaNoiseVar);
      ssaNoiseVariables.insert(std::make_pair(operand, ssaNoiseVar));
    }

    // --- Create a continuous noise variable for the result value ---
    if (!ssaNoiseVariables.contains(op->getResult(0))) {
      std::string varName = "NoiseAt_" + nameAndLoc(op) + "_result";
      auto ssaNoiseVar = solver->MakeNumVar(0, MAX_NOISE, varName);
      allVariables.push_back(ssaNoiseVar);
      ssaNoiseVariables.insert(std::make_pair(op->getResult(0), ssaNoiseVar));
    }
  });

  // --- Step 3: Fix the noise of "input" values to INITIAL_NOISE ---
  // MakeRowConstraint(lower, upper, name) creates a linear constraint:
  //   lower_bound <= (sum of coeff_i * var_i) <= upper_bound
  //
  // Here we fix n_v = INITIAL_NOISE (=12) by setting both lower and upper
  // bounds to the same value.  The constraint is:  1 * n_v == 12.
  //
  // We apply this to:
  //   (a) Block arguments (function parameters in this toy dialect)
  //   (b) Results of EncodeOp (the initial encoding that sets noise = 12)
  // In a real compiler, we'd need a more precise analysis.  Since this
  // tutorial has no control flow, block arguments == function inputs.
  //
  // Example: our MLIR snippet has two encode ops:
  //     %0 = noisy.encode ...    ->  n_0 == 12  ✓
  //     %1 = noisy.encode ...    ->  n_1 == 12  ✓
  for (auto item : ssaNoiseVariables) {
    auto value = item.first;
    auto var = item.second;
    if (isa<BlockArgument>(value) ||
        isa<noisy::EncodeOp>(value.getDefiningOp())) {
      // Constraint:  1 * n_v  == INITIAL_NOISE   (both lb and ub = 12)
      MPConstraint *const ct =
          solver->MakeRowConstraint(INITIAL_NOISE, INITIAL_NOISE, "");
      ct->SetCoefficient(var, 1);
    }
  }

  // --- Step 4: Define constraints linking decision variables and noise ---
  // For MulOp, the noise model is:  result_noise = lhs_noise + rhs_noise
  //   (noise adds linearly for multiplication).
  //
  // For AddOp/SubOp, the noise model is:  result_noise = max(lhs_noise,
  //   rhs_noise) + 1  (noise takes the max plus one).
  //
  // When we insert a reduce_noise op (d_i = 1), the result noise is reset to
  // INITIAL_NOISE (=12) regardless.  So we need a conditional:
  //
  //   result_noise = f(lhs,rhs)   if d_i = 0
  //   result_noise = 12            if d_i = 1
  //
  // where f is the noise combination function for that op type.
  //
  // Since we can't write "if-then-else" in linear programming, we use the
  // "big-M" technique (M = IF_THEN_AUX = 100).  The idea is the same for
  // both op types — the only difference is HOW each op computes "input_noise"
  // from its operands:
  //
  //   MulOp:     input_noise = lhs + rhs   (substituted directly into ct3/ct4
  //              as coefficients — no separate variable needed)
  //
  //   Add/SubOp: input_noise = max(lhs,rhs)+1  (needs an auxiliary Z variable,
  //              since max() is not a linear expression)
  //
  // The four big-M constraints per op are:
  //
  //   (ct1) result >= 12 * d              -> force to 12 when d=1
  //   (ct2) result <= 12 + (1-d) * M      -> allows >12 when d=0
  //   (ct3) result >= input_noise - d*M  -> force to input_noise when d=0
  //   (ct4) result <= input_noise + d*M  -> force to input_noise when d=0
  //
  // When d=0: (ct1) is trivial, (ct2) allows up to 12+M,
  //           (ct3)+(ct4) together force result = input_noise.  ✓
  // When d=1: (ct1)+(ct2) force result = 12,
  //           (ct3)+(ct4) are trivial.  ✓
  //
  //   (ct5) input_noise <= MAX_NOISE   -> safety constraint
  std::string cstName;
  op->walk([&](Operation *op) {
    llvm::TypeSwitch<Operation &>(*op)
        .Case<noisy::MulOp>([&](auto op) {
          // =================================================================
          // MulOp noise model:  result = lhs + rhs   (when d=0)
          //
          // This is NOT computed by a single line of code. Instead, it's
          // ENCODED in the coefficients of ct3 and ct4 below.  When d=0,
          // these two constraints "pinch" result to equal lhs+rhs:
          //
          //   ct3: 1*result + 0 - 1*lhs - 1*rhs >= 0   =>  result >= lhs+rhs
          //   ct4: 1*result - 0 - 1*lhs - 1*rhs <= 0   =>  result <= lhs+rhs
          //   Together:                                  =>  result = lhs+rhs
          //
          // For self-multiplication (lhs==rhs, e.g. %3 = mul %2, %2), the
          // coefficient is -2 instead of -1, giving result = 2*lhs = lhs+rhs.
          // =================================================================
          auto inf = solver->infinity();
          auto lhsNoiseVar = ssaNoiseVariables.lookup(op.getLhs());
          auto rhsNoiseVar = ssaNoiseVariables.lookup(op.getRhs());
          auto resultNoiseVar = ssaNoiseVariables.lookup(op.getResult());
          auto reduceNoiseDecision = decisionVariables.lookup(op);

          // --- Big-M constraints for MulOp ---
          // (ct1)  result >= INITIAL_NOISE * d
          //   Rewritten as:  1*result + (-12)*d >= 0
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_1";
          MPConstraint *const ct1 =
              solver->MakeRowConstraint(0.0, inf, cstName);
          ct1->SetCoefficient(resultNoiseVar, 1);
          ct1->SetCoefficient(reduceNoiseDecision, -INITIAL_NOISE);

          // (ct2)  result <= INITIAL_NOISE + (1-d) * M
          //   Rewritten as:  1*result + M*d <= INITIAL_NOISE + M
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_2";
          MPConstraint *const ct2 = solver->MakeRowConstraint(
              0.0, INITIAL_NOISE * IF_THEN_AUX, cstName);
          ct2->SetCoefficient(resultNoiseVar, 1);
          ct2->SetCoefficient(reduceNoiseDecision, IF_THEN_AUX);

          // (ct3)  result >= (lhs+rhs) - d*M
          //   This is the "≥ half" of  result = lhs+rhs  (when d=0).
          //   Coefficients:  1*result + M*d + (-1)*lhs + (-1)*rhs >= 0
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_3";
          MPConstraint *const ct3 =
              solver->MakeRowConstraint(0.0, inf, cstName);
          ct3->SetCoefficient(resultNoiseVar, 1);
          ct3->SetCoefficient(reduceNoiseDecision, IF_THEN_AUX);
          if (op.getLhs() == op.getRhs()) {
            ct3->SetCoefficient(lhsNoiseVar, -2);  // self-mul: 2*lhs
          } else {
            ct3->SetCoefficient(lhsNoiseVar, -1);
            ct3->SetCoefficient(rhsNoiseVar, -1);
          }

          // (ct4)  result <= (lhs+rhs) + d*M
          //   This is the "≤ half" of  result = lhs+rhs  (when d=0).
          //   Coefficients:  1*result + (-M)*d + (-1)*lhs + (-1)*rhs <= 0
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_4";
          MPConstraint *const ct4 =
              solver->MakeRowConstraint(-inf, 0.0, cstName);
          ct4->SetCoefficient(resultNoiseVar, 1);
          ct4->SetCoefficient(reduceNoiseDecision, -IF_THEN_AUX);
          if (op.getLhs() == op.getRhs()) {
            ct4->SetCoefficient(lhsNoiseVar, -2);  // self-mul: 2*lhs
          } else {
            ct4->SetCoefficient(lhsNoiseVar, -1);
            ct4->SetCoefficient(rhsNoiseVar, -1);
          }

          // (ct5)  lhs_noise + rhs_noise <= MAX_NOISE (safety)
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_5";
          MPConstraint *const ct5 =
              solver->MakeRowConstraint(0.0, MAX_NOISE, cstName);
          if (op.getLhs() == op.getRhs()) {
            ct5->SetCoefficient(lhsNoiseVar, 2);
          } else {
            ct5->SetCoefficient(lhsNoiseVar, 1);
            ct5->SetCoefficient(rhsNoiseVar, 1);
          }
        })
        .Case<noisy::AddOp, noisy::SubOp>([&](auto op) {
          // =================================================================
          // AddOp/SubOp noise model:  result = max(lhs, rhs) + 1  (when d=0)
          //
          // max() is NOT a linear function, so we can't encode it directly
          // as coefficients like we did for MulOp.  Instead, we introduce an
          // auxiliary variable Z and force it to equal max(lhs,rhs)+1 using
          // two tricks:
          //
          //   (z1)  Z >= lhs + 1     ← lower bound from lhs
          //   (z2)  Z >= rhs + 1     ← lower bound from rhs
          //   (obj) minimize 0.1*Z   ← pushes Z DOWN as low as possible
          //
          // Since Z wants to be as small as possible (objective coefficient
          // 0.1), but must be ≥ both (lhs+1) and (rhs+1), the solver will
          // set Z = max(lhs, rhs) + 1  — the tightest lower bound wins.
          //
          // Then ct3+ct4 use Z as the "input_noise" for the big-M pattern,
          // exactly like MulOp used lhs+rhs coefficients directly.
          // =================================================================
          auto inf = solver->infinity();
          auto lhsNoiseVar = ssaNoiseVariables.lookup(op.getLhs());
          auto rhsNoiseVar = ssaNoiseVariables.lookup(op.getRhs());
          auto resultNoiseVar = ssaNoiseVariables.lookup(op.getResult());
          auto reduceNoiseDecision = decisionVariables.lookup(op);

          // --- Same big-M constraints as MulOp (ct1, ct2): control d_i ---
          // (ct1)  result >= INITIAL_NOISE * d
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_1";
          MPConstraint *const ct1 =
              solver->MakeRowConstraint(0.0, inf, cstName);
          ct1->SetCoefficient(resultNoiseVar, 1);
          ct1->SetCoefficient(reduceNoiseDecision, -INITIAL_NOISE);

          // (ct2)  result <= INITIAL_NOISE + (1-d) * M
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_2";
          MPConstraint *const ct2 = solver->MakeRowConstraint(
              0.0, INITIAL_NOISE * IF_THEN_AUX, cstName);
          ct2->SetCoefficient(resultNoiseVar, 1);
          ct2->SetCoefficient(reduceNoiseDecision, IF_THEN_AUX);

          // --- Z := max(lhs_noise, rhs_noise) + 1  (encoded by z1+z2+obj) ---
          // Z is a continuous variable.  The objective wants it small, but
          // z1 and z2 force it ≥ each operand+1, so Z = max(lhs,rhs)+1.
          cstName = "Z_" + nameAndLoc(op);
          auto zVar = solver->MakeNumVar(0, MAX_NOISE, cstName);
          allVariables.push_back(zVar);
          objective->SetCoefficient(zVar, 0.1);

          // (z1)  Z >= lhs_noise + 1
          //   Rewritten as:  1*Z - 1*lhs >= 1
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_z1";
          MPConstraint *const zCt1 =
              solver->MakeRowConstraint(1.0, inf, cstName);
          zCt1->SetCoefficient(zVar, 1);
          zCt1->SetCoefficient(lhsNoiseVar, -1);

          // (z2)  Z >= rhs_noise + 1 (only if operands differ)
          if (op.getLhs() != op.getRhs()) {
            cstName = "DecisionDynamics_" + nameAndLoc(op) + "_z2";
            MPConstraint *const zCt2 =
                solver->MakeRowConstraint(1.0, inf, cstName);
            zCt2->SetCoefficient(zVar, 1);
            zCt2->SetCoefficient(rhsNoiseVar, -1);
          }

          // --- Big-M constraints using Z as input_noise ---
          // (ct3)  result >= Z - d*M
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_3";
          MPConstraint *const ct3 =
              solver->MakeRowConstraint(0.0, inf, cstName);
          ct3->SetCoefficient(resultNoiseVar, 1);
          ct3->SetCoefficient(reduceNoiseDecision, IF_THEN_AUX);
          ct3->SetCoefficient(zVar, -1);

          // (ct4)  result <= Z + d*M
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_4";
          MPConstraint *const ct4 =
              solver->MakeRowConstraint(-inf, 0.0, cstName);
          ct4->SetCoefficient(resultNoiseVar, 1);
          ct4->SetCoefficient(reduceNoiseDecision, -IF_THEN_AUX);
          ct4->SetCoefficient(zVar, -1);

          // (ct5)  Z <= MAX_NOISE  (safety: input noise must not overflow)
          cstName = "DecisionDynamics_" + nameAndLoc(op) + "_5";
          MPConstraint *const ct5 =
              solver->MakeRowConstraint(0.0, MAX_NOISE, cstName);
          ct5->SetCoefficient(zVar, 1);
        });
  });

  // --- Step 5: Solve the MIP and extract the solution ---
  // solver->Solve() runs the SCIP solver on the model we've built.  It finds
  // values for all variables (d_i, n_v) that satisfy all constraints while
  // minimizing the sum of d_i (the total number of reduce_noise ops).
  //
  // The solver communicates with an external SCIP process (OR-Tools handles
  // this transparently).  After solving:
  //   - objective->Value()   returns the optimal objective value
  //   - var->solution_value() returns the optimal value for each variable
  // Uncomment to dump the full LP model in human-readable format for
  // debugging (only useful if you're familiar with LP file format):
  // std::string modelAsString;
  // solver->ExportModelAsLpFormat(false, &modelAsString);
  // LLVM_DEBUG(llvm::dbgs() << "Model string = " << modelAsString << "\n");

  solver->Solve();
  LLVM_DEBUG(llvm::dbgs() << "Problem solved in " << solver->wall_time()
                          << " milliseconds"
                          << "\n");

  LLVM_DEBUG(llvm::dbgs() << "Solution:\n");
  LLVM_DEBUG(llvm::dbgs() << "Objective value = " << objective->Value()
                          << "\n");
  // Uncomment to see every variable's value:
  // LLVM_DEBUG(llvm::dbgs() << "Variables:\n");
  // for (auto var : allVariables) {
  //   LLVM_DEBUG(llvm::dbgs() << "  " << var->name() << " = "
  //                           << var->solution_value() << "\n");
  // }

  // --- Step 6: Extract the solution into our result map ---
  // solution_value() returns the value the solver found for each decision
  // variable d_i.  Since d_i is binary (0 or 1), the result is 0.0 or 1.0,
  // which we store as `bool` in DenseMap<Operation*, bool>.
  // Later, shouldInsertReduceNoise(op) will query this map.
  for (auto item : decisionVariables) {
    solution.insert(std::make_pair(item.first, item.second->solution_value()));
  }
}

} // namespace tutorial
} // namespace mlir
