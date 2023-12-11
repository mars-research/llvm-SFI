#include "llvm/Transforms/IPO/CheriIRPass.h"
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

#define DEBUG_TYPE "Cheriir"

auto cheri_check_func_name = "cheri_cap_store_0";

PreservedAnalyses CheriIRPass::run(Module &M, ModuleAnalysisManager &AM) {

    llvm::Type *Int8ptr = Type::getInt8PtrTy(M.getContext());
    llvm::Type *Int8ptr_cap = Type::getInt8PtrTy(M.getContext(),200);
    llvm::Function *FMemWrtChk;
    llvm::Function * Chk = M.getFunction(cheri_check_func_name);
    if (Chk) {
      FMemWrtChk =  Chk;
    }
    else{
    llvm::Type *Rty = Type::getVoidTy(M.getContext());;
    std::vector<llvm::Type *> PArray = {Int8ptr,Int8ptr_cap};
    llvm::FunctionType *FTy = llvm::FunctionType::get(Rty, PArray, false);
    FMemWrtChk = llvm::Function::Create(FTy, llvm::GlobalValue::ExternalLinkage,
        llvm::Twine(cheri_check_func_name), &M);
    }

    for (Function &F : M){

        if (F.getName() == cheri_check_func_name)
            continue;

        for (auto &BB : F){
            for (auto &I : BB) {
                if(I.getOpcode() == Instruction::Store){
                    I.print(errs());
                    errs()<<"\n";
                    llvm::StoreInst *ST = llvm::dyn_cast<StoreInst>(&I);
                    PointerType *ValuePointerTy = dyn_cast<PointerType>(ST->getValueOperand()->getType());
                    if (ValuePointerTy && ValuePointerTy->getAddressSpace() == 200) {
                        errs()<<ValuePointerTy->getAddressSpace();
                        errs()<<"\n";
                        // ST->getPointerOperand()->print(errs());
                        errs()<<"\n";

                        IRBuilder<> IRB(&(I));
                        std::vector<llvm::Value *> Args;
                        llvm::Value * casted_dest_ptr =  IRB.CreatePointerCast(ST->getPointerOperand() , Int8ptr);
                        llvm::Value * casted_value_ptr =  IRB.CreatePointerCast(ST->getValueOperand() , Int8ptr_cap);
                        Args.push_back(casted_dest_ptr);
                        Args.push_back(casted_value_ptr);
                        llvm::CallInst * cap_check =  IRB.CreateCall(FMemWrtChk, Args);
                    }
                }

            }
        }
 }

    return PreservedAnalyses::all();
}