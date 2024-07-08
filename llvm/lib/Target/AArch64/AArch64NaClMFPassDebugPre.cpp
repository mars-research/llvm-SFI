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
#include <llvm/Support/FileSystem.h>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>

using namespace llvm;

static bool debug = false;

namespace{
    class AArch64NaClMFPassDebugPre : public MachineFunctionPass{
    public:
        static char ID;
        AArch64NaClMFPassDebugPre() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "AArch64NaClMFPassDebugPre"; }
        bool runOnMachineFunction(MachineFunction &MF) override;
    };
}
char AArch64NaClMFPassDebugPre::ID = 0;
bool AArch64NaClMFPassDebugPre::runOnMachineFunction(MachineFunction &MF) {
  //errs()<<"AArch64NaClMFPassDebugPre invoked!\n";
  if(debug){
  std::error_code EC;
  llvm::raw_fd_ostream OS("/home/xiangd/ffmpeg-arm-nacl/MI.nosfi", EC,llvm::sys::fs::OF_Append| llvm::sys::fs::OF_TextWithCRLF); 
  MF.print(OS);
  OS.close();
  }

  return true;
}
FunctionPass *llvm::createAArch64NaClMFPassDebugPre() { 
  //  std::error_code EC;
  // //  llvm::raw_fd_ostream OS("/home/xiangd/ffmpeg-arm-nacl/MI.nosfi", EC);
  //  OS.close();
  return new AArch64NaClMFPassDebugPre(); }
