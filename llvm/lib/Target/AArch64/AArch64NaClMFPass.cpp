//===- //arm nacl -------===//
//
//
//===----------------------------------------------------------------------===//

#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64Subtarget.h"
#include "MCTargetDesc/AArch64AddressingModes.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DebugCounter.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>

using namespace llvm;

namespace{
    class AArch64NaClMFPass : public MachineFunctionPass{
    public:
        bool testing = true;
        static char ID;
        AArch64NaClMFPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "AArch64NaClMFPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}
char AArch64NaClMFPass::ID = 0;
bool AArch64NaClMFPass::runOnMachineFunction(MachineFunction &MF) {
    errs()<<"NaClMFPass invoked!\n";

    for (MachineBasicBlock &MBB : MF) {

      bool sp_used = false;

      const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();

      for (MachineInstr &MI : MBB) {
        if (MI.isDebugInstr() || MI.isCFIInstruction()|| MI.isKill())
          continue;         

        for(auto op = MI.operands_begin(); op != MI.operands_end(); op++){
          if(op->isReg()){
            if(op->getReg() == AArch64::SP){
              op->setReg(AArch64::X15);
              sp_used = true;
            }
          }
        }

        if (!AArch64InstrInfo::isLdSt(MI))
          continue;

        int op_idx; 
        bool is_pairldst = false;
        bool sp_involved = false;
        if (AArch64InstrInfo::isUpdateLdSt(MI)){
            op_idx = 2;
        }else{
            op_idx = 1;
        }
        if (AArch64InstrInfo::isPairLdSt(MI)){
          op_idx += 1;
          is_pairldst = true;
        }else{
          is_pairldst = false;
        }
        if (!MI.getOperand(op_idx).isReg()){
            errs()<<"fatal LDP/STP with no register operand!\n;";
            abort();
        }

          if (MI.getOperand(op_idx).isReg()){
            if (MI.getOperand(op_idx+1).isReg()){
              if(testing){
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  MI.getOperand(op_idx).getReg()).addImm(
                  0).addImm(
                  63);//inject tag
                
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                  MI.getOperand(op_idx+1).getReg()).addReg(
                  MI.getOperand(op_idx+1).getReg()).addReg(
                  AArch64::WZR).addImm(
                  56).addImm(
                  4);//clear tag
              }
              else{
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  AArch64::X27).addImm(
                  56).addImm(
                  4);//inject tag
                  BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                  MI.getOperand(op_idx+1).getReg()).addReg(
                  MI.getOperand(op_idx+1).getReg()).addReg(
                  AArch64::WZR).addImm(
                  56).addImm(
                  4);//clear tag
              }
            }
            else{
              if(testing){
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  MI.getOperand(op_idx).getReg()).addImm(
                  0).addImm(
                  63);//inject tag
              }
              else{
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  MI.getOperand(op_idx).getReg()).addReg(
                  AArch64::X27).addImm(
                  56).addImm(
                  4);//inject tag
              }
            }
          }
        
    }

    if (sp_used){
      for (MachineInstr &MI : MBB) {
        if (MI.isReturn() || MI.isTerminator() || MI.isCall() || MI.isBranch() || MI.isBarrier()){
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::ADDXri)).addReg(
            AArch64::SP).addReg(
            AArch64::X15).addImm(
            0).addImm(
            0);
        }
      }
      
      BuildMI(MBB, MBB.begin(), NULL, TII->get(AArch64::ADDXri)).addReg(
        AArch64::X15).addReg(
        AArch64::SP).addImm(
        0).addImm(
        0);

      BuildMI(MBB, MBB.end(), NULL, TII->get(AArch64::ADDXri)).addReg(
        AArch64::SP).addReg(
        AArch64::X15).addImm(
        0).addImm(
        0);
    }
  }
      return true;
}
FunctionPass *llvm::createAArch64NaClMFPass() { return new AArch64NaClMFPass(); }
