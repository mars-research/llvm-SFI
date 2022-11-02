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
        static char ID;
        AArch64NaClMFPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "AArch64NaClMFPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}
char AArch64NaClMFPass::ID = 0;
bool AArch64NaClMFPass::runOnMachineFunction(MachineFunction &MF) {
    //errs()<<"NaClMFPass invoked!\n";
    for (MachineBasicBlock &MBB : MF) {
      for (MachineInstr &MI : MBB) {
        if (MI.isDebugInstr())
          continue;

        if (AArch64InstrInfo::isLdSt(MI)){
          //errs()<<"found ldst\n";
          //MI.print(errs());
          const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::HINT)).addImm(0);
          //errs()<<"\n";
      }
    }
  }
      return true;
}
FunctionPass *llvm::createAArch64NaClMFPass() { return new AArch64NaClMFPass(); }
