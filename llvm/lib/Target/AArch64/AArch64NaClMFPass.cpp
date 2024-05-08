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
        bool fake_checks = true;
        bool ignore_rrldst = true;
        static char ID;
        AArch64NaClMFPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "AArch64NaClMFPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}
char AArch64NaClMFPass::ID = 0;
bool AArch64NaClMFPass::runOnMachineFunction(MachineFunction &MF) {
    //errs()<<"NaClMFPass invoked!\n";
    // MF.setAlignment(Align(16));
    for (MachineBasicBlock &MBB : MF) {
      // MBB.setAlignment(Align(16));
      int bundle_counter = 0;
      for (MachineInstr &MI : MBB) {
        if (MI.isDebugInstr() || MI.isCFIInstruction()|| MI.isKill())
          continue;          
        
        const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
        if (AArch64InstrInfo::isLdSt(MI)){
          continue;
          int op_idx; 
          if (AArch64InstrInfo::isUpdateLdSt(MI)){
            op_idx = 2;
          }else{
            op_idx = 1;
          }
          if(AArch64InstrInfo::isPairLdSt(MI)){
            op_idx += 1;
          }
          if (!MI.getOperand(op_idx).isReg()){
            errs()<<"fatal LDP/STP with no register operand!\n;";
            abort();
          }
          if (MI.getOperand(op_idx+1).isReg() && !ignore_rrldst){
            errs()<<"two indexing reg ld/st is not supported yet\n;";
            abort();
          }
          else{
            if (bundle_counter==12){
              BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
              bundle_counter = 0;
            }
            if(fake_checks){
              BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                MI.getOperand(op_idx).getReg()).addReg(
                MI.getOperand(op_idx).getReg()).addReg(
                MI.getOperand(op_idx).getReg()).addImm(
                0).addImm(
                63);
            }
            else{
              BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
                MI.getOperand(op_idx).getReg()).addReg(
                MI.getOperand(op_idx).getReg()).addReg(
                AArch64::X28).addImm(
                32).addImm(
                31);
            }
              bundle_counter += 4;
          }
        } //we don't do anything for the tail return, but the call checks will check tail returns
        else if (MI.isReturn() && (MI.getOpcode()!=AArch64::TCRETURNdi) && (MI.getOpcode()!=AArch64::TCRETURNri) && (MI.getOpcode()!=AArch64::TCRETURNriALL) && (MI.getOpcode()!=AArch64::TCRETURNriBTI)){
          // if (bundle_counter==12){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 0;
          // }
          // if (bundle_counter==8){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 0;
          // }

          //emitting return edge checks
          if(fake_checks){
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              AArch64::WZR).addImm(0).addImm(0);//clear
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addImm(0).addImm(63);//inject
          }else{
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              AArch64::WZR).addImm(0).addImm(3);//clear
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              AArch64::X28).addImm(32).addImm(31);//inject 
          }
          bundle_counter += 8;
      }
       else if (MI.isCall()){
        if (MI.getOpcode() == AArch64::BR || MI.getOpcode() == AArch64::BLR){
          // if (bundle_counter==12){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 4;
          // }
          // else if (bundle_counter==8){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 4;
          // }
          // else if (bundle_counter==0){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 4;
          // }

          //emitting indirect branch checks
          if(fake_checks){
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
             AArch64::WZR).addImm(0).addImm(0);//clear
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addImm(0).addImm(63);//inject
          }else{
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              AArch64::WZR).addImm(0).addImm(3);//clear
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              AArch64::X28).addImm(32).addImm(31);//inject 
          }
          bundle_counter += 8;
        }else{
          // if (bundle_counter==8){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 12;
          // }
          // else if (bundle_counter==4){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 12;
          // }
          // else if (bundle_counter==0){
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //   bundle_counter = 12;
          // }
        }
       }
      bundle_counter += 4;
      bundle_counter = bundle_counter%16;
    }
  }
      return true;
}
FunctionPass *llvm::createAArch64NaClMFPass() { return new AArch64NaClMFPass(); }
