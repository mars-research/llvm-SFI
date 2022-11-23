#include "llvm/Transforms/IPO/NaClIRPass.h"
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

#define DEBUG_TYPE "naclir"

PreservedAnalyses NaClIRPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {

    std::error_code EC;


    for (Function &F : M){
            // llvm::raw_fd_ostream OS("/users/BUXD/llvm-SFI/ll.nosfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
            // F.print(OS);
            // OS.close();
        for (auto &BB : F){
            for (auto &I : BB) {
                if(I.getOpcode() == Instruction::Load){

                    //emitting startbundle 
                    IRBuilder<> IRSB(&(I));
                    StringRef SBconstraints = "~{dirflag},~{fpsr},~{flags}";          
                    llvm::Type *voidty = Type::getVoidTy(M.getContext());;
                    std::vector<llvm::Type *> SBPArray = {};
                    llvm::FunctionType *BundleTy = llvm::FunctionType::get(voidty, SBPArray, false);
                    StringRef SBAsmString = "mov %NaclStartBundle, %NaclStartBundle";
                    llvm::InlineAsm *SBIA = llvm::InlineAsm::get(BundleTy,SBAsmString,SBconstraints,true,InlineAsm::AD_ATT); 
                    ArrayRef<Value *> BundleArgs = {};
                    llvm::CallInst *SBResult = IRSB.CreateCall(SBIA, BundleArgs);
                    SBResult->setDebugLoc(I.getDebugLoc());

                    //emitting rewritting
                    IRBuilder<> IRB(&(I));
                    StringRef constraints = "=r,0,~{dirflag},~{fpsr},~{flags}";
                    llvm::LoadInst *LD = llvm::dyn_cast<LoadInst>(&I);
                    llvm::Type *Rty = LD->getPointerOperand()->getType();
                    std::vector<llvm::Type *> PArray = {LD->getPointerOperand()->getType()};
                    llvm::FunctionType *FTy = llvm::FunctionType::get(Rty, PArray, false);
                    StringRef AsmString = "or $$0x0, $0;or $0, $0;";
                    llvm::InlineAsm *IA = llvm::InlineAsm::get(FTy,AsmString,constraints,true,InlineAsm::AD_ATT); 
                    ArrayRef<Value *> Args = {LD->getPointerOperand()};
                    llvm::CallInst *Result = IRB.CreateCall(IA, {LD->getPointerOperand()});
                    I.getOperandList()[0]=Result;
                    Result->setDebugLoc(I.getDebugLoc());

                    //emitting endbundle
                    IRBuilder<> IRBE(I.getNextNonDebugInstruction());           
                    StringRef EBconstraints = "~{dirflag},~{fpsr},~{flags}";      
                    std::vector<llvm::Type *> EBPArray = {};
                    //llvm::FunctionType *BundleTy = llvm::FunctionType::get(voidty, EBPArray, false);
                    StringRef EBAsmString = "mov %NaclEndBundle, %NaclEndBundle";
                    llvm::InlineAsm *EBIA = llvm::InlineAsm::get(BundleTy,EBAsmString,EBconstraints,true,InlineAsm::AD_ATT); 
                    ArrayRef<Value *> EBBundleArgs = {};
                    llvm::CallInst *EBResult = IRBE.CreateCall(EBIA, EBBundleArgs);
                    EBResult->setDebugLoc(I.getDebugLoc());
                }else if(I.getOpcode() == Instruction::Store){

                    //emitting startbundle 
                    IRBuilder<> IRSB(&(I));
                    StringRef SBconstraints = "~{dirflag},~{fpsr},~{flags}";          
                    llvm::Type *voidty = Type::getVoidTy(M.getContext());;
                    std::vector<llvm::Type *> SBPArray = {};
                    llvm::FunctionType *BundleTy = llvm::FunctionType::get(voidty, SBPArray, false);
                    StringRef SBAsmString = "mov %NaclStartBundle, %NaclStartBundle";
                    llvm::InlineAsm *SBIA = llvm::InlineAsm::get(BundleTy,SBAsmString,SBconstraints,true,InlineAsm::AD_ATT); 
                    ArrayRef<Value *> BundleArgs = {};
                    llvm::CallInst *SBResult = IRSB.CreateCall(SBIA, BundleArgs);
                    SBResult->setDebugLoc(I.getDebugLoc());


                    llvm::StoreInst *ST = llvm::dyn_cast<StoreInst>(&I);
                    IRBuilder<> IRB(ST);
                    llvm::Type *Rty = ST->getPointerOperand()->getType();
                    std::vector<llvm::Type *> PArray = {Rty};
                    llvm::FunctionType *FTy = llvm::FunctionType::get(Rty, PArray, false);
                    StringRef AsmString = "or $$0x0, $0;or $0, $0;";
                    StringRef constraints = "=r,0,~{dirflag},~{fpsr},~{flags}";
                    llvm::InlineAsm *IA = llvm::InlineAsm::get(FTy,AsmString,constraints,true,InlineAsm::AD_ATT); 
                    ArrayRef<Value *> Args = {ST->getPointerOperand()};
                    llvm::CallInst *Result = IRB.CreateCall(IA, {ST->getPointerOperand()});
                    I.getOperandList()[1]=Result;
                    Result->setDebugLoc(I.getDebugLoc());
                    //errs()<<"end of store\n";

                    //emitting endbundle
                    IRBuilder<> IRBE(I.getNextNonDebugInstruction());           
                    StringRef EBconstraints = "~{dirflag},~{fpsr},~{flags}";      
                    std::vector<llvm::Type *> EBPArray = {};
                    //llvm::FunctionType *BundleTy = llvm::FunctionType::get(voidty, EBPArray, false);
                    StringRef EBAsmString = "mov %NaclEndBundle, %NaclEndBundle";
                    llvm::InlineAsm *EBIA = llvm::InlineAsm::get(BundleTy,EBAsmString,EBconstraints,true,InlineAsm::AD_ATT); 
                    ArrayRef<Value *> EBBundleArgs = {};
                    llvm::CallInst *EBResult = IRBE.CreateCall(EBIA, EBBundleArgs);
                    EBResult->setDebugLoc(I.getDebugLoc());
                }
            }
        }
        // llvm::raw_fd_ostream OSS("/users/BUXD/llvm-SFI/ll.sfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
        // F.print(OSS);
        // OSS.close();
    }

    return PreservedAnalyses::all();
}