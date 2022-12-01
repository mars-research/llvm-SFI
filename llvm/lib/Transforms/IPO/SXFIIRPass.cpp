#include "llvm/Transforms/IPO/SXFIIRPass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
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

#define DEBUG_TYPE "SXFIir"

static bool debug = false;

PreservedAnalyses SXFIIRPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {

    std::error_code EC;

    llvm::Type *Int8ptr = Type::getInt8PtrTy(M.getContext());
    llvm::Function *FMemWrtChk;
    llvm::Function * Chk = M.getFunction("sxfi_capability_check");
    if (Chk) {
      FMemWrtChk =  Chk;
    }
    else{
    llvm::Type *Rty = Type::getVoidTy(M.getContext());;
    std::vector<llvm::Type *> PArray = {Int8ptr};
    llvm::FunctionType *FTy = llvm::FunctionType::get(Rty, PArray, false);
    FMemWrtChk = llvm::Function::Create(FTy, llvm::GlobalValue::ExternalLinkage,
        llvm::Twine("sxfi_capability_check"), &M);
    }
    for (Function &F : M){
            llvm::raw_fd_ostream OS("ll.nosfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
            F.print(OS);
            OS.close();
        std::vector<BasicBlock *> BBs;
        for (BasicBlock &BB : F)
            BBs.push_back(&BB);

            // BBs.size() will change within the loop, so we query it every time
        for (unsigned i = 0; i < BBs.size(); i++) {
            BasicBlock &BB = *BBs[i];
            for (Instruction &I : BB) {
                if(I.getOpcode() == Instruction::Load && !I.SXFI_rewritten){
                    
                    ///*****
                    if(debug){
                        errs()<<"\n\nfind a load: ;\n";
                        I.print(errs());
                        errs()<<"\n before: \n";
                        F.print(errs());
                    }
                    ///*****


                    //create bb for slowpath fast path stay in the old bb
                    BasicBlock* slowpathBB = SplitBlock(I.getParent(),&I);


                    ///*****
                    if(debug){
                        errs()<<"\n ---------after----------: \n";
                        F.print(errs());
                        errs()<<"\nend \n\n\n;\n";
                    }
                    ///*****


                    IRBuilder<> IRB(&(I));
                    llvm::LoadInst *LD = llvm::dyn_cast<LoadInst>(&I);
                    std::vector<llvm::Value *> Args;
                    Args.push_back(LD->getPointerOperand());
                    llvm::CallInst * cap_check =  IRB.CreateCall(FMemWrtChk, Args);


                    //create bb for load
                    BasicBlock* normal_bb = SplitBlock(I.getParent(),&I);


                    //create inline fast path call
                    llvm::Instruction * branch_inst = BB.getTerminator();
                    if(branch_inst){
                        if(branch_inst->getOpcode() != Instruction::Br){
                            errs()<<"fatal split BB terminator is not Br\n";  
                        }
                    }else{
                        errs()<<"fatal split BB has not terminator\n";
                        abort();
                    }
                    IRBuilder<> Builder(branch_inst);
                    llvm::Type *voidty = Type::getVoidTy(M.getContext());
                    std::vector<llvm::Type *> PArray = {};
                    PArray.push_back(Builder.getPtrTy());
                    PArray.push_back(Builder.getPtrTy());
                    PArray.push_back(Builder.getPtrTy());
                    llvm::FunctionType *FastPathFTy = llvm::FunctionType::get(voidty, PArray, false);
                    StringRef AsmString = "mov $1, $0;shr $$32, $0;cmpq %r15, $0;jne ${2:l};1:;";
                    StringRef constraints = "r,r,X,~{ax},~{dirflag},~{fpsr},~{flags}";
                    llvm::BlockAddress * BA = BlockAddress::get(normal_bb);
                    llvm::InlineAsm *IA = llvm::InlineAsm::get(FastPathFTy,AsmString,constraints,true,InlineAsm::AD_ATT); 
                    std::vector<Value *> IAArgs = {};
                    IAArgs.push_back(UndefValue::get(Builder.getPtrTy()));
                    IAArgs.push_back(LD->getPointerOperand());
                    IAArgs.push_back(BA);
                    llvm::CallBrInst *Result =Builder.CreateCallBr(IA, slowpathBB, normal_bb, IAArgs);
                    //end of fast path call

                    I.SXFI_rewritten = true;
                    i=0;
                    BBs.clear();
                    for (BasicBlock &BB : F)
                        BBs.push_back(&BB);
                    break;
                }else if(I.getOpcode() == Instruction::Store && !I.SXFI_rewritten){

                    ///*****
                    if(debug){
                        errs()<<"\n\nfind a store: ;\n";
                        I.print(errs());
                        errs()<<"\n before: \n";
                        F.print(errs());
                    }
                    ///*****


                    //create bb for slowpath fast path stay in the old bb
                    BasicBlock* slowpathBB = SplitBlock(I.getParent(),&I);


                    ///*****
                    if(debug){
                        errs()<<"\n ---------after----------: \n";
                        F.print(errs());
                        errs()<<"\nend \n\n\n;\n";
                    }
                    ///*****


                    IRBuilder<> IRB(&(I));
                    llvm::StoreInst *ST = llvm::dyn_cast<StoreInst>(&I);
                    std::vector<llvm::Value *> Args;
                    Args.push_back(ST->getPointerOperand());
                    llvm::CallInst * cap_check =  IRB.CreateCall(FMemWrtChk, Args);


                    //create bb for load
                    BasicBlock* normal_bb = SplitBlock(I.getParent(),&I);


                    //create inline fast path call
                    llvm::Instruction * branch_inst = BB.getTerminator();
                    if(branch_inst){
                        if(branch_inst->getOpcode() != Instruction::Br){
                            errs()<<"fatal split BB terminator is not Br\n";  
                        }
                    }else{
                        errs()<<"fatal split BB has not terminator\n";
                        abort();
                    }
                    IRBuilder<> Builder(branch_inst);
                    llvm::Type *voidty = Type::getVoidTy(M.getContext());
                    std::vector<llvm::Type *> PArray = {};
                    PArray.push_back(Builder.getPtrTy());
                    PArray.push_back(Builder.getPtrTy());
                    PArray.push_back(Builder.getPtrTy());
                    llvm::FunctionType *FastPathFTy = llvm::FunctionType::get(voidty, PArray, false);
                    StringRef AsmString = "mov $1, $0;shr $$32, $0;cmpq %r15, $0;jne ${2:l};1:;";
                    StringRef constraints = "r,r,X,~{ax},~{dirflag},~{fpsr},~{flags}";
                    llvm::BlockAddress * BA = BlockAddress::get(normal_bb);
                    llvm::InlineAsm *IA = llvm::InlineAsm::get(FastPathFTy,AsmString,constraints,true,InlineAsm::AD_ATT); 
                    std::vector<Value *> IAArgs = {};
                    IAArgs.push_back(UndefValue::get(Builder.getPtrTy()));
                    IAArgs.push_back(ST->getPointerOperand());
                    IAArgs.push_back(BA);
                    llvm::CallBrInst *Result =Builder.CreateCallBr(IA, slowpathBB, normal_bb, IAArgs);
                    //end of fast path call

                    I.SXFI_rewritten = true;
                    i=0;
                    BBs.clear();
                    for (BasicBlock &BB : F)
                        BBs.push_back(&BB);
                    break;
                }else if(I.getOpcode() == Instruction::Call && !I.SXFI_rewritten){
                    llvm::CallInst *Call = llvm::dyn_cast<CallInst>(&I);
                    if(!Call->isIndirectCall())
                        continue;

                    IRBuilder<> IRB(&(I));
                    std::vector<llvm::Value *> Args;
                    Args.push_back(Call->getOperand(0));
                    llvm::CallInst * cap_check =  IRB.CreateCall(FMemWrtChk, Args);

                    I.SXFI_rewritten = true;
                }else if(I.getOpcode() == Instruction::IndirectBr && !I.SXFI_rewritten){
                    llvm::IndirectBrInst *IBr = llvm::dyn_cast<IndirectBrInst>(&I);

                    IRBuilder<> IRB(&(I));
                    std::vector<llvm::Value *> Args;
                    Args.push_back(IBr->getOperand(0));
                    llvm::CallInst * cap_check =  IRB.CreateCall(FMemWrtChk, Args);

                    I.SXFI_rewritten = true;
                }
            }
        }
        llvm::raw_fd_ostream OSS("ll.sfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
        F.print(OSS);
        OSS.close();
    }
    return PreservedAnalyses::all();
}