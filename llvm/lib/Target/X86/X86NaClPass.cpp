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
    class X86NaClPass : public MachineFunctionPass{
    public:
        static char ID;
        llvm::raw_fd_ostream *OS;
        X86NaClPass() : MachineFunctionPass(ID){}
        StringRef getPassName() const override { return "X86NaClPass"; }
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

char X86NaClPass::ID = 0;
bool X86NaClPass::runOnMachineFunction(MachineFunction &MF) {
    //Function boundary
    MF.setAlignment(Align(32));

    for (MachineBasicBlock &MBB : MF) {
      //Block boundary
      MBB.setAlignment(Align(32));

      const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
      LivePhysRegs LiveRegs(*RegInfo);
      LiveRegs.addLiveOuts(MBB);
      bool Emitting_Rewrite_PushPop = false;
      for (MachineInstr &MI : MBB) {
        //MI.print(errs());
        SmallVector<std::pair<MCPhysReg, const MachineOperand *>, 8> Clobbers;
        LiveRegs.stepForward(MI,Clobbers);

        //These instruction will not be complied anyways
        if (MI.isDebugInstr() || MI.isCFIInstruction()|| MI.isKill()||MI.isInlineAsm() || MI.isBarrier())
          continue;        
        
        const DebugLoc DL = MI.getDebugLoc();
        const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

             
        //enforce RSP before push/pop
        if(isPopPush(MI)){
          #ifndef NaCl_LDST_CHECK  
          continue;
          #endif
          // count the number of push and pop in a row
          // we can put at least 8 push and pop in the same bundle safely. 
          if(Emitting_Rewrite_PushPop == false){
            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr)) 
            .addReg(X86::RSP, RegState::Define)
            .addReg(X86::RSP, RegState::Kill)
            .addImm(0x0);

            BuildMI(MBB, MI, DL, TII->get(X86::AND64rr))
            .addReg(X86::RSP, RegState::Define)
            .addReg(X86::RSP, RegState::Kill)
            .addReg(X86::RSP, RegState::Kill);
            Emitting_Rewrite_PushPop = true;
          }
        }
        else if(MI.mayLoad() || MI.mayStore() || skip(MI)){
          Emitting_Rewrite_PushPop = false;
          #ifndef NaCl_LDST_CHECK  
          continue;
          #endif
          //some weird instructions will load and branch at the same time, skip them
          if(MI.isCall() || MI.isBranch())
            continue;

          bool Rip_ins = false; 
          for (const auto &Op : MI.operands()){
            if(Op.isReg()){
              if(Op.getReg() == X86::RIP){
                Rip_ins = true;
              }
            }
          }
          if(Rip_ins)
            continue;


            int numofop = MI.getNumOperands();
            if(numofop <= 1)
              continue;
            const MCInstrDesc &Desc = MI.getDesc();
            int MemRefBeginIdx = X86II::getMemoryOperandNo(Desc.TSFlags);
            MemRefBeginIdx += X86II::getOperandBias(Desc);
            // MI.print(errs());
            // errs()<<MI.getNumOperands()<<"\n";
            // if(MI.getNumOperands() <= 4)
            //   continue;
            MachineOperand &Dest =    MI.getOperand(0);
            MachineOperand &Base =    MI.getOperand(MemRefBeginIdx + X86::AddrBaseReg);
            // MachineOperand &Scale =   MI.getOperand(MemRefBeginIdx + X86::AddrScaleAmt);
            // MachineOperand &Index =   MI.getOperand(MemRefBeginIdx + X86::AddrIndexReg);
            // MachineOperand &Offset =  MI.getOperand(MemRefBeginIdx + X86::AddrDisp);
            // MachineOperand &Segment = MI.getOperand(MemRefBeginIdx + X86::AddrSegmentReg);
          //   errs()<<"Dest: "<<Dest<<"\n";
          //   errs()<<"Base: "<<Base<<"\n";
          //   errs()<<"Scale: "<<Scale<<"\n";
          //   errs()<<"Index: "<<Index<<"\n";
          //  errs()<<"Offset: "<<Offset<<"\n";
          //   errs()<<"Segment: "<<Segment<<"\n\n";

            
            // if(!Base.isReg()){
            //   errs()<<"\n err "<<MI;
            //   continue;
            // }
            // if(!Index.isReg()){
            //   errs()<<"\n err "<<MI;
            //   continue;
            // }
            // if(!Scale.isImm()){
            //   errs()<<"\n err "<<MI;
            //   continue;
            // }
            // if(!Offset.isImm()){
            //   errs()<<"\n err "<<MI;
            //   continue;
            // }

            //skip rip relative load/store
            // if(Base.getReg() == X86::RIP || Index.getReg() == X86::RIP)
            //   continue;

          //   bool base_reg = true;
          //   bool index_reg = true;
          //   if(Base.getReg() == X86::NoRegister)
          //     base_reg = false;
          //   if(Index.getReg() == X86::NoRegister)
          //     index_reg = false;
          // //   errs()<<"Dest: "<<Dest<<"\n";
          // //   errs()<<"Base: "<<Base<<"\n";
          // //   errs()<<"base_reg: "<<base_reg<<"\n";
          // //   errs()<<"Scale: "<<Scale<<"\n";
          // //   errs()<<"Index: "<<Index<<"\n";
          // //   errs()<<"index_reg: "<<index_reg<<"\n";
          // //  errs()<<"Offset: "<<Offset<<"\n";
          // //   errs()<<"Segment: "<<Segment<<"\n\n";
          //   if(base_reg && !index_reg){
          //     BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr)) 
		      //      .addReg(Base.getReg(), RegState::Define)
          //      .addReg(Base.getReg(), RegState::Kill)
          //      .addImm(0x0)
          //      .setMIFlags(MachineInstr::NaclStartBundle);

          //     Index.setReg(X86::R15);
          //     Scale.setImm(1);
          //     MI.setFlag(MachineInstr::NaclEndBundle);
          //     continue;
          //   }
            // if(!base_reg && index_reg){
            //   BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr)) 
		        //   .addReg(Index.getReg(), RegState::Define)
            //   .addReg(Index.getReg(), RegState::Kill)
            //   .addImm(0x0)
            //   .setMIFlags(MachineInstr::NaclStartBundle);

            //   // Base.setReg(Index.getReg());
            //   // Index.setReg(X86::R15);
            //   // Scale.setImm(1);
            //   MI.setFlag(MachineInstr::NaclEndBundle);
            //   continue;
            // }

            // BuildMI(MBB, MI, DL, TII->get(X86::LEA64r), X86::R10)
		        // .addReg(Base.getReg()) //base
            // .addImm(Scale.getImm())
            // .addReg(Index.getReg())
            // .addImm(Offset.getImm())
            // .addReg(Segment.getReg())
            // .setMIFlags(MachineInstr::NaclStartBundle);

            // BuildMI(MBB, MI, DL, TII->get(X86::LEA64r), X86::R10)
		        // .addReg(Base.getReg()) //base
            // .addImm(Scale.getImm())
            // .addReg(Index.getReg())
            // .addImm(Offset.getImm())
            // .addReg(Segment.getReg())
            // .setMIFlags(MachineInstr::NaclStartBundle);
              // BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr)) 
		          //  .addReg(X86::R10, RegState::Define)
              //  .addReg(X86::R10, RegState::Kill)
              //  .addImm(0x0)
              //  .setMIFlags(MachineInstr::NaclStartBundle);
              if(MI.getNumOperands() > MemRefBeginIdx + X86::AddrBaseReg){
              BuildMI(MBB, MI, DL, TII->get(X86::LEA64r), Base.getReg())
              .addReg(Base.getReg()) //base
              .addImm(1)
              .addReg(X86::NoRegister)
              .addImm(0)
              .addReg(X86::NoRegister)
              .setMIFlags(MachineInstr::NaclStartBundle);
              }else{
              BuildMI(MBB, MI, DL, TII->get(X86::LEA64r), X86::R10)
              .addReg(X86::R15) //base
              .addImm(1)
              .addReg(X86::R10)
              .addImm(0)
              .addReg(X86::NoRegister)
              .setMIFlags(MachineInstr::NaclStartBundle);
              }

            
            
            // Base.setReg(X86::R10);
            // Scale.setImm(1);
            // Index.setReg(X86::R15);
            // Offset.setImm(0);
            // Segment.setReg(0);
            MI.setFlag(MachineInstr::NaclEndBundle);
            //errs()<<"after :"<<MI;
        }
        else if(MI.getOpcode()==X86::RET64){
          Emitting_Rewrite_PushPop = false;
          #ifndef NaCl_RETCALL_CHECK  
          continue;
          #endif
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
          Emitting_Rewrite_PushPop = false;
          #ifndef NaCl_RETCALL_CHECK  
          continue;
          #endif
          const DebugLoc DL = MI.getDebugLoc();
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
          
        BuildMI(MBB, MI, DL, TII->get(X86::OR64ri32))
            .addReg(MI.getOperand(0).getReg(), RegState::Define)
            .addReg(MI.getOperand(0).getReg(), RegState::Kill)
            .addImm(0x0)
            .setMIFlags(MachineInstr::NaclStartBundle);
        BuildMI(MBB, MI, DL, TII->get(X86::AND64rr))
            .addReg(MI.getOperand(0).getReg(), RegState::Define)
            .addReg(MI.getOperand(0).getReg(), RegState::Kill)
            .addReg(MI.getOperand(0).getReg(), RegState::Kill);
        MI.setFlag(MachineInstr::NaclEndBundle);
        }else{
          Emitting_Rewrite_PushPop = false;
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
