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
        if (MI.isReturn() && (MI.getOpcode()!=AArch64::TCRETURNdi) && (MI.getOpcode()!=AArch64::TCRETURNri) && (MI.getOpcode()!=AArch64::TCRETURNriALL) && (MI.getOpcode()!=AArch64::TCRETURNriBTI)){
          //emitting return edge checks
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              AArch64::WZR).addImm(0).addImm(0);//clear
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addImm(0).addImm(63);//inject

      }
       else if (MI.isCall()){
        if (MI.getOpcode() == AArch64::BR || MI.getOpcode() == AArch64::BLR){
          //emitting indirect branch checks
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
             AArch64::WZR).addImm(0).addImm(0);//clear
            BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::BFMXri)).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addReg(
              MI.getOperand(0).getReg()).addImm(0).addImm(63);//inject
       }
    }
  }
      return true;
}
FunctionPass *llvm::createAArch64NaClMFPass() { return new AArch64NaClMFPass(); }
