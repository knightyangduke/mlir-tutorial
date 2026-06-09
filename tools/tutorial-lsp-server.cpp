//===- tutorial-lsp-server.cpp - MLIR Language Server for Poly & Noisy -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lib/Dialect/Noisy/NoisyDialect.h"
#include "lib/Dialect/Poly/PolyDialect.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Tools/lsp-server-support/Protocol.h"
#include "mlir/Tools/mlir-lsp-server/MlirLspServerMain.h"

using namespace mlir;

int main(int argc, char **argv) {
  DialectRegistry registry;
  registerAllDialects(registry);
  registry.insert<mlir::tutorial::poly::PolyDialect>();
  registry.insert<mlir::tutorial::noisy::NoisyDialect>();

  return failed(MlirLspServerMain(argc, argv, registry));
}
