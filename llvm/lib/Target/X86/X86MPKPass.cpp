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

static bool google_nacl = false;


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

static bool isPopPush(MachineInstr &MI){
      switch (MI.getOpcode()) {
    case X86::POP16r:
    case X86::POP16rmm:
    case X86::POP16rmr:
    case X86::POPF16:
    case X86::POPA16:
    case X86::POPDS16:
    case X86::POPES16:
    case X86::POPFS16:
    case X86::POPGS16:
    case X86::POPSS16:
    case X86::POP32r:
    case X86::POP32rmm:
    case X86::POP32rmr:
    case X86::POPA32:
    case X86::POPDS32:
    case X86::POPES32:
    case X86::POPF32:
    case X86::POPFS32:
    case X86::POPGS32:
    case X86::POPSS32:
    case X86::POP64r:
    case X86::POP64rmm:
    case X86::POP64rmr:
    case X86::POPF64:
    case X86::POPFS64:
    case X86::POPGS64:
    case X86::PUSH16i8:
    case X86::PUSH16r:
    case X86::PUSH16rmm:
    case X86::PUSH16rmr:
    case X86::PUSHA16:
    case X86::PUSHCS16:
    case X86::PUSHDS16:
    case X86::PUSHES16:
    case X86::PUSHF16:
    case X86::PUSHFS16:
    case X86::PUSHGS16:
    case X86::PUSHSS16:
    case X86::PUSHi16:
    case X86::PUSH32i8:
    case X86::PUSH32r:
    case X86::PUSH32rmm:
    case X86::PUSH32rmr:
    case X86::PUSHA32:
    case X86::PUSHCS32:
    case X86::PUSHDS32:
    case X86::PUSHES32:
    case X86::PUSHF32:
    case X86::PUSHFS32:
    case X86::PUSHGS32:
    case X86::PUSHSS32:
    case X86::PUSHi32:
    case X86::PUSH64i32:
    case X86::PUSH64i8:
    case X86::PUSH64r:
    case X86::PUSH64rmm:
    case X86::PUSH64rmr:
    case X86::PUSHF64:
    case X86::PUSHFS64:
    case X86::PUSHGS64:
      return true;  
    }
    return false;
}

static bool skip(MachineInstr &MI){
  switch (MI.getOpcode()) {
    case X86::MOV8mr_NOREX:
    case X86::MOV8rm_NOREX:
    case X86::MOV8rr_NOREX:
    case X86::MOVSX32rm8_NOREX:
    case X86::MOVSX32rr8_NOREX:
    case X86::MOVZX32rm8_NOREX:
    case X86::MOVZX32rr8_NOREX:
    case X86::MOV8mr:
    case X86::MOV8rm:
    case X86::MOV8rr:
    case X86::MOVSX32rm8:
    case X86::MOVSX32rr8:
    case X86::MOVZX32rm8:
    case X86::MOVZX32rr8:
    case X86::TEST8ri:
      return true;
    }
  return false;
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
