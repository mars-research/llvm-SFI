#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;

/// Pass to convert @llvm.global.annotations to !annotation metadata.
struct PACIRPass : public PassInfoMixin<PACIRPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm