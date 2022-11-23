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
#include "llvm/Support/raw_ostream.h"
#include <llvm/Support/FileSystem.h>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
using namespace llvm;


namespace{
    class X86NaClDebugPostPass : public MachineFunctionPass{
    public:
        static char ID;
        llvm::raw_fd_ostream *OS;
        X86NaClDebugPostPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "X86NaClDebugPostPass"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}
char X86NaClDebugPostPass::ID = 0;
bool X86NaClDebugPostPass::runOnMachineFunction(MachineFunction &MF) {

  std::error_code EC;
  llvm::raw_fd_ostream OS("/users/BUXD/x86-ffmpeg-nacl/MI.sfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
  MF.print(OS);
  OS.close();
   return true;
}
FunctionPass *llvm::createX86NaClDebugPostPass() { 
  //  std::error_code EC;
  // //  llvm::raw_fd_ostream OS(
  // //        "/users/BUXD/llvm-SFI/MI.sfi", EC);
  // OS.close();
  return new X86NaClDebugPostPass(); 
  }
