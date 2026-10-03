#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define MemSize \
  1000  // memory size, in reality, the memory size should be 2^32, but for this
        // lab csa23, for the space resaon, we keep it as this large number, but
        // the memory is still 32-bit addressable.

struct IFStruct {
  bitset<32> PC;
  bool nop;
};

struct IDStruct {
  bitset<32> Instr;
  bool nop;
};

struct EXStruct {
  bitset<32> Read_data1;
  bitset<32> Read_data2;
  bitset<16> Imm;
  bitset<5> Rs;
  bitset<5> Rt;
  bitset<5> Wrt_reg_addr;
  bool is_I_type;
  bool rd_mem;
  bool wrt_mem;
  bool alu_op;  // 1 for addu, lw, sw, 0 for subu
  bool wrt_enable;
  bool nop;
};

struct MEMStruct {
  bitset<32> ALUresult;
  bitset<32> Store_data;
  bitset<5> Rs;
  bitset<5> Rt;
  bitset<5> Wrt_reg_addr;
  bool rd_mem;
  bool wrt_mem;
  bool wrt_enable;
  bool nop;
};

struct WBStruct {
  bitset<32> Wrt_data;
  bitset<5> Rs;
  bitset<5> Rt;
  bitset<5> Wrt_reg_addr;
  bool wrt_enable;
  bool nop;
};

struct stateStruct {
  IFStruct IF;
  IDStruct ID;
  EXStruct EX;
  MEMStruct MEM;
  WBStruct WB;
};

class RF {
 public:
  bitset<32> Reg_data;
  RF() {
    Registers.resize(32);
    Registers[0] = bitset<32>(0);
  }

  bitset<32> readRF(bitset<5> Reg_addr) {
    Reg_data = Registers[Reg_addr.to_ulong()];
    return Reg_data;
  }

  void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
    Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
  }

  void outputRF(int cycle) {
    ofstream rfout;
    rfout.open("RFresult.txt", std::ios_base::app);
    if (rfout.is_open()) {
      rfout << "State of RF (Cycle=" << cycle << ")\t" << endl;

      for (int j = 0; j < 32; j++) {
        rfout << Registers[j] << endl;
      }
    } else
      cout << "Unable to open file";
    rfout.close();
  }

 private:
  vector<bitset<32> > Registers;
};

class INSMem {
 public:
  bitset<32> Instruction;
  INSMem() {
    IMem.resize(MemSize);
    ifstream imem;
    string line;
    int i = 0;
    imem.open("imem.txt");
    if (imem.is_open()) {
      while (getline(imem, line)) {
        IMem[i] = bitset<8>(line);
        i++;
      }
    } else
      cout << "Unable to open file";
    imem.close();
  }

  bitset<32> readInstr(bitset<32> ReadAddress) {
    string insmem;
    insmem.append(IMem[ReadAddress.to_ulong()].to_string());
    insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
    insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
    insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
    Instruction = bitset<32>(insmem);  // read instruction memory
    return Instruction;
  }

 private:
  vector<bitset<8> > IMem;
};

class DataMem {
 public:
  bitset<32> ReadData;
  DataMem() {
    DMem.resize(MemSize);
    ifstream dmem;
    string line;
    int i = 0;
    dmem.open("dmem.txt");
    if (dmem.is_open()) {
      while (getline(dmem, line)) {
        DMem[i] = bitset<8>(line);
        i++;
      }
    } else
      cout << "Unable to open file";
    dmem.close();
  }

  bitset<32> readDataMem(bitset<32> Address) {
    string datamem;
    datamem.append(DMem[Address.to_ulong()].to_string());
    datamem.append(DMem[Address.to_ulong() + 1].to_string());
    datamem.append(DMem[Address.to_ulong() + 2].to_string());
    datamem.append(DMem[Address.to_ulong() + 3].to_string());
    ReadData = bitset<32>(datamem);  // read data memory
    return ReadData;
  }

  void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
    DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
    DMem[Address.to_ulong() + 1] =
        bitset<8>(WriteData.to_string().substr(8, 8));
    DMem[Address.to_ulong() + 2] =
        bitset<8>(WriteData.to_string().substr(16, 8));
    DMem[Address.to_ulong() + 3] =
        bitset<8>(WriteData.to_string().substr(24, 8));
  }

  void outputDataMem() {
    ofstream dmemout;
    dmemout.open("dmemresult.txt");
    if (dmemout.is_open()) {
      for (int j = 0; j < 1000; j++) {
        dmemout << DMem[j] << endl;
      }
    } else
      cout << "Unable to open file";
    dmemout.close();
  }

 private:
  vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle) {
  ofstream printstate;
  printstate.open("stateresult.txt", std::ios_base::app);
  if (printstate.is_open()) {
    printstate << "State after executing cycle:\t" << cycle << endl;

    printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
    printstate << "IF.nop:\t" << state.IF.nop << endl;

    printstate << "ID.Instr:\t" << state.ID.Instr << endl;
    printstate << "ID.nop:\t" << state.ID.nop << endl;

    printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
    printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
    printstate << "EX.Imm:\t" << state.EX.Imm << endl;
    printstate << "EX.Rs:\t" << state.EX.Rs << endl;
    printstate << "EX.Rt:\t" << state.EX.Rt << endl;
    printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
    printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
    printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
    printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
    printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
    printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
    printstate << "EX.nop:\t" << state.EX.nop << endl;

    printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
    printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
    printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
    printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
    printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
    printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
    printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
    printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
    printstate << "MEM.nop:\t" << state.MEM.nop << endl;

    printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
    printstate << "WB.Rs:\t" << state.WB.Rs << endl;
    printstate << "WB.Rt:\t" << state.WB.Rt << endl;
    printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
    printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
    printstate << "WB.nop:\t" << state.WB.nop << endl;
  } else
    cout << "Unable to open file";
  printstate.close();
}

int main() {
  RF myRF;
  INSMem myInsMem;
  DataMem myDataMem;
  stateStruct state, newState;
  int cycle = 0;
  int opCode = 0;
  int funct = 0;
  bitset<5> rs;
  bitset<5> rt;
  bitset<5> rd;
  bitset<5> shamt;
  bitset<16> imm;
  bool stall = false;
  bool branch = false;
  std::bitset<32> branchAddress(0);

  while (1) {
    /* --------------------- WB stage --------------------- */
    // Write back to RF
    if (!state.WB.nop && state.WB.wrt_enable == 1) {
      myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
    }

    /* --------------------- MEM stage --------------------- */
    if (!state.MEM.nop) {
      newState.WB.Rs = state.MEM.Rs;
      newState.WB.Rt = state.MEM.Rt;
      newState.WB.wrt_enable = state.MEM.wrt_enable;
      newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
      newState.WB.nop = false;
      if (state.MEM.wrt_mem && !state.WB.nop && state.WB.wrt_enable &&
          state.WB.Wrt_reg_addr == state.MEM.Rt) {
        myDataMem.writeDataMem(state.MEM.ALUresult, state.WB.Wrt_data);
        newState.WB.Wrt_data = state.MEM.ALUresult;
      } else if (state.MEM.wrt_mem == 1) {
        myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
        newState.WB.Wrt_data = state.MEM.ALUresult;
      } else if (state.MEM.rd_mem == 1) {
        newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
      } else {
        newState.WB.Wrt_data = state.MEM.ALUresult;
      }
    } else {
      newState.WB.nop = 1;
    }

    /* --------------------- EX stage --------------------- */
    if (!state.EX.nop) {
      //    // come back to this, perform nop
      
      //   } if (state.EX.alu_op == 1) { what is this for i dont think we need
      //   anything outside the !nop and nop

    } else {
      newState.MEM.nop = 1;
    }

    /* --------------------- ID stage --------------------- */
    // check if ID stage nop is set to 0
    if (!state.ID.nop) {
      // decode instruction

      // set opCode
      opCode = stoi(state.ID.Instr.to_string().substr(0, 6), nullptr, 2);

      // get rs and rt
      rs = stoi(state.ID.Instr.to_string().substr(6, 5), nullptr, 2);

      // get rs and rt
      rt = stoi(state.ID.Instr.to_string().substr(11, 5), nullptr, 2);

      // get rs and rt
      rd = stoi(state.ID.Instr.to_string().substr(16, 5), nullptr, 2);

      // get rs and rt
      shamt = stoi(state.ID.Instr.to_string().substr(21, 5), nullptr, 2);

      // set funct
      funct = stoi(state.ID.Instr.to_string().substr(26, 6), nullptr, 2);

      // set imm
      imm = stoi(state.ID.Instr.to_string().substr(16, 16), nullptr, 2);

      // update newState values for EX
      newState.EX.Rs = rs;
      newState.EX.Rt = rt;
      newState.EX.Read_data1 = myRF.readRF(rs);
      newState.EX.Read_data2 = myRF.readRF(rt);
      newState.EX.nop = 0;
      newState.EX.Imm = bitset<16>(imm);

      // for Addu and Subu
      if (opCode == 0x00) {
        // update destination
        newState.EX.Wrt_reg_addr = rd;

        // set I_type
        newState.EX.is_I_type = 0;

        // set rd_mem
        newState.EX.rd_mem = 0;

        // set rd_mem
        newState.EX.wrt_mem = 0;

        // set rd_mem
        newState.EX.wrt_enable = 1;

        // checks for funct
        if (funct == 0x21) {
          // set aluop to 1 for addu
          newState.EX.alu_op = 1;

        } else if (funct == 0x23) {
          // set aluop to 0 for subu
          newState.EX.alu_op = 0;
        }
      } else if (opCode == 0x23 || opCode == 0x2B) {
        // set aluop to 1 for lw or sw
        newState.EX.alu_op = 1;

        // update destination
        newState.EX.Wrt_reg_addr = rt;

        // update I_type
        newState.EX.is_I_type = 1;

        if (opCode == 0x23) {
          // lw instruction
          // set read memory
          newState.EX.rd_mem = 1;

          // set write memory
          newState.EX.wrt_mem = 0;

          // set write memory
          newState.EX.wrt_enable = 1;

        } else if (opCode == 0x2B) {
          // sw instruction
          // set read memory
          newState.EX.rd_mem = 0;

          // set write memory
          newState.EX.wrt_mem = 1;

          // set write memory
          newState.EX.wrt_enable = 0;
        }

      } else if (opCode == 0x05) {
        // bne instruction type

        // compare both read data
        if (newState.EX.Read_data1 != newState.EX.Read_data2) {
          branch = true;
          branchAddress =
              bitset<32>(state.IF.PC.to_ulong() +
                         (((short)newState.EX.Imm.to_ulong()) << 2));
        }

        // set read memory
        newState.EX.rd_mem = 0;

        // set write memory
        newState.EX.wrt_mem = 0;

        // set write memory
        newState.EX.wrt_enable = 0;

        // set EX newstate to 0
        newState.EX.nop = 1;

        // HALT instruction, shouldn't be called tho, already handled in IF
      } else if (opCode == 0x3f) {
        // Halt instruction type
        //  set read memory
        newState.EX.rd_mem = 0;

        // set write memory
        newState.EX.wrt_mem = 0;

        // set write memory
        newState.EX.wrt_enable = 0;
      }

      // ID.NOP = 1
    } else {
      // set nop of EX to 1
      newState.EX.nop = 1;
    }

    // checks if current inst. in EX is lw and it is not nop
    if (!state.ID.nop && state.EX.rd_mem && !state.EX.nop && state.EX.Wrt_reg_addr != 0) {
      // check if current inst in ID is addu or subu
      if (opCode == 0x00) {
        // check if state EX Wrt_reg_addr is equal to rs or rt
        if (state.EX.Wrt_reg_addr == rs || state.EX.Wrt_reg_addr == rt) {
            // stall
            stall = true;
        }

        // check if current inst in ID is lw
      } else if (opCode == 0x23) {
        // check if state EX Wrt_reg_addr is equal to rs or rt
        if (state.EX.Wrt_reg_addr == rs) {
            // stall
            stall = true;
        }

        // check if current inst in ID is sw
      } else if (opCode == 0x2B) {
        // check if state EX Wrt_reg_addr is equal to rs or rt
        if (state.EX.Wrt_reg_addr == rs || state.EX.Wrt_reg_addr == rt) {
            // stall
            stall = true;
        }
      }
    }

    /* --------------------- IF stage --------------------- */
    if (!state.IF.nop) {
      // branch
      if (branch) {
        newState.EX.nop = 1;
        branch = false;
        newState.ID = state.ID;
        newState.IF.PC = branchAddress;
        newState.ID.nop = 1;

        // stall
      } else if (stall) {
        newState.EX.nop = 1;
        stall = false;
        newState.IF = state.IF;
        newState.ID = state.ID;

        // normal fetch is not branch or stall
      } else {
        // updated new state for ID
        newState.ID.nop = 0;
        newState.IF.nop = 0;

        // Fetch instruction from memory
        newState.ID.Instr = myInsMem.readInstr(state.IF.PC);

        // get opcode
        opCode = stoi(newState.ID.Instr.to_string().substr(0, 6), nullptr, 2);

        // update PC
        newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);

        // checks if the instr is a HALT instruction
        if (opCode == 0x3f) {
          // set new state of IF to nop
          newState.IF.nop = 1;

          // set new state of ID to nop
          newState.ID.nop = 1;
        }
      }

    } else {
      // set new state of nop to 1, indiciating that the next stage of ID
      // should perform a nop
      newState.ID.nop = 1;
    }

    if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop &&
        state.WB.nop)
      break;

    printState(
        newState,
        cycle);  // print states after executing cycle 0, cycle 1, cycle 2 ...

    state = newState; /*** The end of the cycle and updates the current state
                         with the values calculated in this cycle. csa23 ***/

    myRF.outputRF(cycle);  // dump RF;

    cycle++;
  }

  myDataMem.outputDataMem();  // dump data mem

  return 0;
}

// a06ab8e681fd21c64d696c93bec4b1ef