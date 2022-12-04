#include "llvm/Transforms/IPO/PACIRPass.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/Support/FileSystem.h>
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/InstrTypes.h"
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

using namespace llvm;


PreservedAnalyses PACIRPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
    errs()<<"PACIRPass invoked!\n";

    std::error_code EC;


    for (Function &F : M){
        errs()<<"hello \n";
            // llvm::raw_fd_ostream OS("/users/BUXD/llvm-SFI/ll.nosfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
            // F.print(OS);
            // OS.close();
        for (auto &BB : F){
            for (auto &I : BB) {
                if(I.getOpcode() == Instruction::Load){
                }else if(I.getOpcode() == Instruction::Store){
                }
            }
        }
        // llvm::raw_fd_ostream OSS("/users/BUXD/llvm-SFI/ll.sfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
        // F.print(OSS);
        // OSS.close();
    }

    return PreservedAnalyses::all();
}