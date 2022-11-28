//===- //SXFI -------===//
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
    class X86SXFIPass : public MachineFunctionPass{
    public:
        static char ID;
        llvm::raw_fd_ostream *OS;
        X86SXFIPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "X86SXFIPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}

char X86SXFIPass::ID = 0;
bool X86SXFIPass::runOnMachineFunction(MachineFunction &MF) {
      //MF.setAlignment(Align(32));
    for (MachineBasicBlock &MBB : MF) {
      //MBB.setAlignment(Align(32));
      int bundle_counter = 0;
      for (MachineInstr &MI : MBB) {
        if (MI.isDebugInstr() || MI.isCFIInstruction()|| MI.isKill())
          continue;        
        
        if(MI.getOpcode()==X86::RET64){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
          BuildMI(&MBB, DL, TII->get(X86::MOV64rm), X86::R14)
            .addReg(0)
            .addImm(1)
            .addReg(0)
            .addImm(0)
            .addReg(X86::R8);
          BuildMI(MBB, MI, DL, TII->get(X86::SUB64ri8))
            .addReg(X86::R14, RegState::Define)
            .addReg(X86::R14, RegState::Kill)
            .addImm(0x8);
          BuildMI(MBB, MI, DL, TII->get(X86::POP64r)) 
		        .addReg(X86::R8, RegState::Define);
          BuildMI(MBB, MI, DL, TII->get(X86::JMP64r))
            .addReg(X86::R8, RegState::Kill);
        }
        else if(MI.getOpcode()==X86::CALL64pcrel32 || MI.getOpcode()==X86::CALL64r){
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

            errs()<<MI.getOperand(0).getType()<<"\n";
            if (MI.getOperand(0).getType() == MachineOperand::MO_GlobalAddress){
              MI.getOperand(0).getGlobal()->print(errs());
              const Constant* c = MI.getOperand(0).getGlobal();
              const llvm::Function  *gf = llvm::dyn_cast<Function>(c);
              if(gf){
                if(gf->getName() == "sxfi_capability_check"){
                  continue;
                }
              }
                
            }else if(MI.getOperand(0).getType() == MachineOperand::MO_ExternalSymbol){
              continue;
            }

          BuildMI(MBB, MI, DL, TII->get(X86::ADD64ri8))
            .addReg(X86::R14, RegState::Define)
            .addReg(X86::R14, RegState::Kill)
            .addImm(0x8);
          int imm = (MI.getOpcode()==X86::CALL64pcrel32) ? 5 : 2;
          imm += 3;
          BuildMI(MBB, MI, DL, TII->get(X86::LEA64r))
            .addReg(X86::RAX, RegState::Define)
            .addReg(X86::RIP)
            .addImm(1)
            .addReg(0)
            .addImm(imm)
            .addReg(0);
          addRegOffset(BuildMI(MBB, MI, DL, TII->get(X86::MOV64mr)),X86::R14, false, 0)
		        .addReg(X86::RAX);
        }
      }
    }
  return true;
}
FunctionPass *llvm::createX86SXFIPass() { 

  return new X86SXFIPass(); 
  }
