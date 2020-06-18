#pragma once

#include "instruction.h"
#include "cop0.h"

class PSX;

class CPU {
public:
    CPU(PSX* system);
    void RunInstruction();
    void DecodeAndExecute(uint32_t instruction);
private:
    PSX* system = nullptr;
    cop0 COP0;

    uint32_t registers[32];
    uint32_t PC, hi, lo;

    uint32_t next_inst;

    bool is_pending_load;
    uint32_t pending_reg, pending_load_data;

    // Should be called after an instruction is decoded but before writing
    void ExecutePendingLoad();
    void Branch(int imm);

    void HandleCop0(const Instruction& inst);

    void sll(const Instruction& inst);
    void jr(const Instruction& inst);
    void addu(const Instruction& inst);
    void and_(const Instruction& inst);
    void or_(const Instruction& inst);
    void xor_(const Instruction& inst);
    void nor(const Instruction& inst);
    void sltu(const Instruction& inst);

    void j(const Instruction& inst);
    void jal(const Instruction& inst);
    void beq(const Instruction& inst);
    void bne(const Instruction& inst);
    void addi(const Instruction& inst);
    void addiu(const Instruction& inst);
    void andi(const Instruction& inst);
    void ori(const Instruction& inst);
    void xori(const Instruction& inst);
    void lui(const Instruction& inst);
    void mtc0(const Instruction& inst);
    void lb(const Instruction& inst);
    void lh(const Instruction& inst);
    void lw(const Instruction& inst);
    void sb(const Instruction& inst);
    void sh(const Instruction& inst);
    void sw(const Instruction& inst);
};

