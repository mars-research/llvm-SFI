//===- //NaCl -------===//
//
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86Subtarget.h"
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
#include "X86AsmPrinter.h"
#include <llvm/Support/FileSystem.h>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>

using namespace llvm;


namespace{
    class X86NaClPass : public MachineFunctionPass{
    public:
        static char ID;
        llvm::raw_fd_ostream *OS;
        X86NaClPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "X86NaClPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}

char X86NaClPass::ID = 0;
bool X86NaClPass::runOnMachineFunction(MachineFunction &MF) {
      MF.setAlignment(Align(32));
    for (MachineBasicBlock &MBB : MF) {
      MBB.setAlignment(Align(32));
      int bundle_counter = 0;
      for (MachineInstr &MI : MBB) {
        if (MI.isDebugInstr() || MI.isCFIInstruction()|| MI.isKill())
          continue;        
        

        if(MI.getOpcode()==X86::RET64){
	        const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
	        BuildMI(MBB, MI, DL, TII->get(X86::POP64r)) 
		        .addReg(X86::R8, RegState::Define)
            .setMIFlags(MachineInstr::NaclStartBundle);//R8 is caller saved so it's ok to use it to store the ret addr
          BuildMI(MBB, MI, DL, TII->get(X86::OR64ri32))
            .addReg(X86::R8, RegState::Define)
            .addReg(X86::R8, RegState::Kill)
            .addImm(0x0);
          BuildMI(MBB, MI, DL, TII->get(X86::AND64rr))
            .addReg(X86::R8, RegState::Define)
            .addReg(X86::R8, RegState::Kill)
            .addReg(X86::R8, RegState::Kill);
          BuildMI(MBB, MI, DL, TII->get(X86::JMP64r))
            .addReg(X86::R8, RegState::Kill)
            .setMIFlags(MachineInstr::NaclEndBundle);
        }else if(MI.getOpcode() == X86::CALL64r || MI.getOpcode() == X86::JMP64r){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
          //errs()<<"call64r\n";
        BuildMI(MBB, MI, DL, TII->get(X86::OR64ri32))
            .addReg(MI.getOperand(0).getReg(), RegState::Define)
            .addReg(MI.getOperand(0).getReg(), RegState::Kill)
            .addImm(0x0)
            .setMIFlags(MachineInstr::NaclStartBundle);
        BuildMI(MBB, MI, DL, TII->get(X86::AND64rr))
            .addReg(MI.getOperand(0).getReg(), RegState::Define)
            .addReg(MI.getOperand(0).getReg(), RegState::Kill)
            .addReg(MI.getOperand(0).getReg(), RegState::Kill);
        MI.setFlags(MachineInstr::NaclEndBundle);
        }else{
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
          int n = MI.getNumOperands();
          for (int i = 0; i < n; i++){
            auto operand = MI.getOperand(i);
            if(operand.isReg()){
              //nacl TODO: add rsp checks
              if(operand.getReg() == X86::RSP && (getRegState(operand) & RegState::Define)){
                //BuildMI(MBB, MI, DL, TII->get(X86::NOOP));
              }
            }
          }
        }
        
      }
    }
  return true;
}
FunctionPass *llvm::createX86NaClPass() { 

  return new X86NaClPass(); 
  }
