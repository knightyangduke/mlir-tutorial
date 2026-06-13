
#include "lib/Utility/CustomPrintIRHandler.hpp"
#include "lib/Utility/DebugActionTag.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Unit.h"
#include "llvm/Support/raw_ostream.h"

// =============================================================================
// 2. Implement the Custom Intercepting Handler
// =============================================================================
void CustomPrintIRHandler::operator()(llvm::function_ref<void()> execute, const mlir::tracing::Action &action) {
    // Write a single, generic handler that scales to ANY tag

    if (debugActionTag.empty() || action.getTag() != debugActionTag) {
      execute();
      return;
    }

    triggerCount++;
    // --- Scalable Debugging Logic ---
    // This block only executes for the exact tag you passed in the terminal!
    // action.print() dispatches to the overridden print() on each derived
    // action type, which includes both the tag and the static desc string.
    llvm::errs() << "[INTERCEPTED] ";
    action.print(llvm::errs());
    llvm::errs() << "\n              Trigger Count: " << triggerCount << "\n";

    // Helper lambda to print operations from the action's IRUnits.
    // The op passed in is already scoped at the call site (e.g. func.func),
    // so no further filtering is needed here.
    auto printFuncOps = [](llvm::ArrayRef<mlir::IRUnit> irUnits) {
      for (const auto &unit : irUnits) {
        if (auto *op = llvm::dyn_cast<mlir::Operation *>(unit)) {
          op->print(llvm::errs());
          llvm::errs() << "\n";
        }
      }
    };

    //Dump current IR before action executes
    llvm::errs() << "--- IR Before ---\n";
    printFuncOps(action.getContextIRUnits());
    
    execute(); 

    //Dump current IR after action executes
    llvm::errs() << "--- IR After ---\n";
    printFuncOps(action.getContextIRUnits());
}
