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

    void sll(const Instruction& inst);
    void or_(const Instruction& inst);
    void j(const Instruction& inst);
    void addiu(const Instruction& inst);
    void ori(const Instruction& inst);
    void lui(const Instruction& inst);
    void mtc0(const Instruction& inst);
    void sw(const Instruction& inst);
};

