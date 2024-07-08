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
#include "llvm/IR/Dominators.h"
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

using namespace llvm;


PreservedAnalyses PACIRPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
    errs()<<"PACIRPass invoked!\n";

    //return PreservedAnalyses::all();
    std::error_code EC;
    llvm::Type *Int64Ty = Type::getInt64Ty(M.getContext());
    llvm::Type *PtrTy = Type::getInt64PtrTy(M.getContext());
    llvm::Type *VoidTy = Type::getVoidTy(M.getContext());
    ArrayType * ArrayTy = ArrayType::get(Int64Ty, 256);

    GlobalValue* metadata_table = M.getNamedValue("metadata_table");
    if(!metadata_table){
        metadata_table = new GlobalVariable(M, 
        /*Type=*/ArrayTy,
        /*isConstant=*/false,
        /*Linkage=*/GlobalValue::ExternalLinkage,
        /*Initializer=*/0,
        /*Name=*/"metadata_table");
    }

    for (Function &F : M){
        errs()<<"hello \n";
            // llvm::raw_fd_ostream OS("/users/BUXD/llvm-SFI/ll.nosfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
            // F.print(OS);
            // OS.close();
        //DominatorTree DT = DominatorTree(F);
        for (auto &BB : F){
            std::unordered_map<Value*, Value*> Ptr_map;
            for (auto &I : BB) {
                if(I.getOpcode() == Instruction::Load){
                    llvm::LoadInst *LD = llvm::dyn_cast<LoadInst>(&I);
                    IRBuilder<> IRB(&(I));
                    Value* Ptr = LD->getPointerOperand();

                    if (auto search = Ptr_map.find(Ptr); search != Ptr_map.end()){
                        // while(search != Ptr_map.end()){
                        //     if()
                        //     search = search.next();
                        // }
                        errs()<<"find a reusble ptr :";
                        Ptr->print(errs());
                        errs()<<"for :";
                        I.print(errs());
                        errs()<<"\n\n";
                        I.getOperandList()[0]=search->second;
                        continue;
                    }
                        

                    //%2 = ptrtoint ptr %0 to i64
                    Value* v2 = IRB.CreatePtrToInt(Ptr,Int64Ty);

                    //%3 = and i64 %2, 281474976710655
                    Value* v3 = IRB.CreateAnd(v2,llvm::ConstantInt::get(Int64Ty,0xFFFFFFFFFFFF));

                    //%4 = inttoptr i64 %3 to ptr ; this is the real value of ptr
                    Value* v4 = IRB.CreateIntToPtr(v3,PtrTy);
                    Ptr_map.insert({Ptr, v4});

                    if(false){
                    errs()<<"\n------hashmap begin-------\n";
                    for( const std::pair<const Value*, Value*>& n : Ptr_map ){ 
                        errs()<<"\nkey: ";
                        n.first->print(errs());
                        errs()<<"\nv: ";
                        n.second->print(errs());
                    }
                    errs()<<"\n------hashmap end-------\n";
                    }

                    //update the load/store ptr
                    I.getOperandList()[0]=v4;

                    //%5 = lshr i64 %2, 48
                    Value* v5 = IRB.CreateLShr(v2, llvm::ConstantInt::get(Int64Ty,48));

                    //%6 = lshr i64 %2, 56
                    Value* v6 = IRB.CreateLShr(v2, llvm::ConstantInt::get(Int64Ty,56));

                    //%7 = shl nsw i64 -1, %6
                    Value* v7 = IRB.CreateShl(llvm::ConstantInt::get(Int64Ty,-1), v6, "", false, true);
                    
                    //%8 = and i64 %7, %2
                    Value* v8 = IRB.CreateAnd(v7,v2);

                    //%9 = inttoptr i64 %8 to ptr
                    Value* v9 = IRB.CreateIntToPtr(v8,PtrTy);

                    // %10 = getelementptr inbounds [65536 x i64], ptr @metadata_table, i64 0, i64 %5
                    Value *v10 = IRB.CreateGEP(ArrayTy, metadata_table,{llvm::ConstantInt::get(Int64Ty,0), v5},"", true);

                    //%11 = load i64, ptr %10, align 8, !tbaa !6
                    Value *v11 = IRB.CreateLoad(Int64Ty, v10);

                    //%12 = tail call ptr asm "autda $0, $2", "=r,0,r"(ptr %9, i64 %11) #5, !srcloc !11
                    StringRef AsmString = "pacda $0, $2";
                    llvm::FunctionType *AsmTy = llvm::FunctionType::get(PtrTy, {PtrTy,Int64Ty}, false);
                    StringRef constraints = "=r,0,r";
                    llvm::InlineAsm *IA = llvm::InlineAsm::get(AsmTy,AsmString,constraints,true); 
                    llvm::CallInst *v12 = IRB.CreateCall(IA, {v9, v11});
                }else if(I.getOpcode() == Instruction::Store){
                    llvm::StoreInst *ST = llvm::dyn_cast<StoreInst>(&I);
                    IRBuilder<> IRB(&(I));
                    Value* Ptr = ST->getPointerOperand();

                    if (auto search = Ptr_map.find(Ptr); search != Ptr_map.end()){
                        errs()<<"find a reusble ptr :";
                        Ptr->print(errs());
                        errs()<<"for :";
                        I.print(errs());
                        errs()<<"\n\n";
                        I.getOperandList()[1]=search->second;
                        continue;
                    }

                    //beginning of bundle
                    // StringRef NopAsmString = "nop";
                    // llvm::FunctionType *NopAsmTy = llvm::FunctionType::get(VoidTy, {}, false);
                    // StringRef Nopconstraints = "";
                    // llvm::InlineAsm *NopIA = llvm::InlineAsm::get(NopAsmTy,NopAsmString,Nopconstraints,true); 
                    // IRB.CreateCall(NopIA);

                    //%2 = ptrtoint ptr %0 to i64
                    Value* v2 = IRB.CreatePtrToInt(Ptr,Int64Ty);

                    //%3 = and i64 %2, 281474976710655
                    Value* v3 = IRB.CreateAnd(v2,llvm::ConstantInt::get(Int64Ty,0xFFFFFFFFFFFF));

                    //%4 = inttoptr i64 %3 to ptr ; this is the real value of ptr
                    Value* v4 = IRB.CreateIntToPtr(v3,PtrTy);
                    Ptr_map.insert({Ptr, v4});

                    //update the load/store ptr
                    I.getOperandList()[1]=v4;

                    //%5 = lshr i64 %2, 48
                    Value* v5 = IRB.CreateLShr(v2, llvm::ConstantInt::get(Int64Ty,48));

                    //%6 = lshr i64 %2, 56
                    Value* v6 = IRB.CreateLShr(v2, llvm::ConstantInt::get(Int64Ty,56));

                    //%7 = shl nsw i64 -1, %6
                    Value* v7 = IRB.CreateShl(llvm::ConstantInt::get(Int64Ty,-1), v6, "", false, true);
                    
                    //%8 = and i64 %7, %2
                    Value* v8 = IRB.CreateAnd(v7,v2);

                    //%9 = inttoptr i64 %8 to ptr
                    Value* v9 = IRB.CreateIntToPtr(v8,PtrTy);

                    // %10 = getelementptr inbounds [65536 x i64], ptr @metadata_table, i64 0, i64 %5
                    Value *v10 = IRB.CreateGEP(ArrayTy, metadata_table,{llvm::ConstantInt::get(Int64Ty,0), v5},"", true);

                    //%11 = load i64, ptr %10, align 8, !tbaa !6
                    Value *v11 = IRB.CreateLoad(Int64Ty, v10);

                    //%12 = tail call ptr asm "autda $0, $2", "=r,0,r"(ptr %9, i64 %11) #5, !srcloc !11
                    StringRef AsmString = "autda $0, $2";
                    llvm::FunctionType *AsmTy = llvm::FunctionType::get(PtrTy, {PtrTy,Int64Ty}, false);
                    StringRef constraints = "=r,0,r";
                    llvm::InlineAsm *IA = llvm::InlineAsm::get(AsmTy,AsmString,constraints,true); 
                    llvm::CallInst *v12 = IRB.CreateCall(IA, {v9, v11});
                }
            }
        }
        // llvm::raw_fd_ostream OSS("/users/BUXD/llvm-SFI/ll.sfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
        // F.print(OSS);
        // OSS.close();
    }

    return PreservedAnalyses::all();
}