#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;

/// Pass to convert @llvm.global.annotations to !annotation metadata.
struct CheriIRPass : public PassInfoMixin<CheriIRPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

}