#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;

/// Pass to convert @llvm.global.annotations to !annotation metadata.
struct SXFIIRPass : public PassInfoMixin<SXFIIRPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm