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
#include <llvm/Support/FileSystem.h>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>

using namespace llvm;


namespace{
    class AArch64PACMFPass : public MachineFunctionPass{
    public:
        static char ID;
        llvm::raw_fd_ostream *OS;
        AArch64PACMFPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "AArch64PACMFPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}
char AArch64PACMFPass::ID = 0;
bool AArch64PACMFPass::runOnMachineFunction(MachineFunction &MF) {
    MachineBasicBlock *first_MBB = MF.getBlockNumbered(0);
    MachineInstr &first_MI = *first_MBB->getFirstNonDebugInstr();
    const TargetInstrInfo *TII = first_MBB->getParent()->getSubtarget().getInstrInfo();
    BuildMI(*first_MBB, first_MI, first_MI.getDebugLoc(), TII->get(AArch64::PACIASP));

    for (MachineBasicBlock &MBB : MF) {
        const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
        for (MachineInstr &MI : MBB) {
            if (MI.isReturn() && (MI.getOpcode()!=AArch64::TCRETURNdi) && (MI.getOpcode()!=AArch64::TCRETURNri) && (MI.getOpcode()!=AArch64::TCRETURNriALL) && (MI.getOpcode()!=AArch64::TCRETURNriBTI)){
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::AUTIASP));
            }
        }
    }
  return true;
}
FunctionPass *llvm::createAArch64PACMFPass() { 
//   std::error_code EC;
//   llvm::raw_fd_ostream OS(
//         "MI.sfi", EC);
//   OS.close();
  return new AArch64PACMFPass(); 
  }