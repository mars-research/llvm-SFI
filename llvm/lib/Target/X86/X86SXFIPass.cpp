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
      MF.setAlignment(Align(32));
    for (MachineBasicBlock &MBB : MF) {
      MBB.setAlignment(Align(32));
      int bundle_counter = 0;
      for (MachineInstr &MI : MBB) {
        if (MI.isDebugInstr() || MI.isCFIInstruction()|| MI.isKill())
          continue;        
        
        
      }
    }
  return true;
}
FunctionPass *llvm::createX86SXFIPass() { 

  return new X86SXFIPass(); 
  }
