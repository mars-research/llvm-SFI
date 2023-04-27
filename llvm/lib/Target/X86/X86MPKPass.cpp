//===- //NaCl -------===//
//
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/X86MCTargetDesc.h"
#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86Subtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LivePhysRegs.h"
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
    class X86MPKPass : public MachineFunctionPass{
    public:
        static char ID;
        llvm::raw_fd_ostream *OS;
        X86MPKPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "X86MPKPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}

char X86MPKPass::ID = 0;
bool X86MPKPass::runOnMachineFunction(MachineFunction &MF) {
    for (MachineBasicBlock &MBB : MF) {
      for (MachineInstr &MI : MBB) {
        if(MI.getOpcode() == X86::CALL64r){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::R10)
		        .addReg(MI.getOperand(0).getReg()) //base
            .addImm(1)
            .addReg(X86::NoRegister)
            .addImm(0)
            .addReg(X86::NoRegister);

        // BuildMI(MBB, MI, DL, TII->get(X86::AND64rr))
        //     .addReg(MI.getOperand(0).getReg(), RegState::Define)
        //     .addReg(MI.getOperand(0).getReg(), RegState::Kill)
        //     .addReg(MI.getOperand(0).getReg(), RegState::Kill);
        }else if(MI.getOpcode() == X86::CALL64m){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
           const MCInstrDesc &Desc = MI.getDesc();
            int MemRefBeginIdx = X86II::getMemoryOperandNo(Desc.TSFlags);
            MemRefBeginIdx += X86II::getOperandBias(Desc);

            MachineOperand &Dest =    MI.getOperand(0);
            MachineOperand &Base =    MI.getOperand(MemRefBeginIdx + X86::AddrBaseReg);
            MachineOperand &Scale =   MI.getOperand(MemRefBeginIdx + X86::AddrScaleAmt);
            MachineOperand &Index =   MI.getOperand(MemRefBeginIdx + X86::AddrIndexReg);
            MachineOperand &Offset =  MI.getOperand(MemRefBeginIdx + X86::AddrDisp);
            MachineOperand &Segment = MI.getOperand(MemRefBeginIdx + X86::AddrSegmentReg);

            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::R10)
		        .addReg(Base.getReg()) //base
            .addImm(Scale.getImm())
            .addReg(Index.getReg())
            .addImm(Offset.getImm())
            .addReg(Segment.getReg());

            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::R10)
		        .addReg(X86::R10) //base
            .addImm(1)
            .addReg(X86::NoRegister)
            .addImm(0)
            .addReg(X86::NoRegister);


        }else if(MI.getOpcode() == X86::JMP64r){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

            BuildMI(MBB, MI, DL, TII->get(X86::PUSH64r), X86::R10);
            
            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::R10)
		        .addReg(MI.getOperand(0).getReg()) //base
            .addImm(1)
            .addReg(X86::NoRegister)
            .addImm(0)
            .addReg(X86::NoRegister);
            
            BuildMI(MBB, MI, DL, TII->get(X86::POP64r), X86::R10);

        }else if(MI.getOpcode() == X86::JMP64m){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
           const MCInstrDesc &Desc = MI.getDesc();
            int MemRefBeginIdx = X86II::getMemoryOperandNo(Desc.TSFlags);
            MemRefBeginIdx += X86II::getOperandBias(Desc);

            MachineOperand &Dest =    MI.getOperand(0);
            MachineOperand &Base =    MI.getOperand(MemRefBeginIdx + X86::AddrBaseReg);
            MachineOperand &Scale =   MI.getOperand(MemRefBeginIdx + X86::AddrScaleAmt);
            MachineOperand &Index =   MI.getOperand(MemRefBeginIdx + X86::AddrIndexReg);
            MachineOperand &Offset =  MI.getOperand(MemRefBeginIdx + X86::AddrDisp);
            MachineOperand &Segment = MI.getOperand(MemRefBeginIdx + X86::AddrSegmentReg);

             BuildMI(MBB, MI, DL, TII->get(X86::PUSH64r), X86::R10);

            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::R10)
		        .addReg(Base.getReg()) //base
            .addImm(Scale.getImm())
            .addReg(Index.getReg())
            .addImm(Offset.getImm())
            .addReg(Segment.getReg());

            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::R10)
		        .addReg(X86::R10) //base
            .addImm(1)
            .addReg(X86::NoRegister)
            .addImm(0)
            .addReg(X86::NoRegister);

            BuildMI(MBB, MI, DL, TII->get(X86::POP64r), X86::R10);
        }
      }
    }
  return true;
}
FunctionPass *llvm::createX86MPKPass() { 

  return new X86MPKPass(); 
  }
