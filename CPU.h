#pragma once

#include <cstdint>

class PSX;

class CPU {
public:
    CPU(PSX* system);
    void RunInstruction();
    void DecodeAndExecute(uint32_t instruction);
private:
    PSX* system = nullptr;

    uint32_t registers[32];
    uint32_t PC, hi, lo;

    union Instruction {
        struct RType {
            uint32_t opcode : 6;
            uint32_t rs : 5;
            uint32_t rt : 5;
            uint32_t rd : 5;
            uint32_t shamt : 5;
            uint32_t funct : 6;
        } Rinst;
        struct IType {
            uint32_t opcode : 6;
            uint32_t rs : 5;
            uint32_t rt : 5;
            uint32_t immediate : 16;
        } Iinst;
        struct JType {
            uint32_t opcode : 6;
            uint32_t address : 26;
        } Jinst;
        uint32_t inst;
    };
};

