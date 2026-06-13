#pragma once

#include "mlir/IR/Action.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLFunctionalExtras.h"

// =============================================================================
// Custom Intercepting Handler
//
// Register with MLIRContext::registerActionHandler.
// Intercepts any action whose tag matches the --my-debug-tag CLI flag, then
// prints the enclosing func.func IR before and after the rewrite fires.
// Pass a different tag value to switch between watching add vs. mul rewrites.
// =============================================================================
class CustomPrintIRHandler {
public:
  CustomPrintIRHandler() : triggerCount(0) {}
  void operator()(llvm::function_ref<void()> execute,
                  const mlir::tracing::Action &action);

private:
  int triggerCount;
};