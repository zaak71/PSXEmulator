#pragma once

#include <cstdint>

struct Instruction {
    uint32_t inst;
    Instruction(uint32_t i) : inst(i) {};
    uint32_t opcode() const { return inst >> 26; }
    uint32_t funct() const { return inst & 0x3F; }
    uint32_t rs() const { return (inst >> 21) & 0x1F; }
    uint32_t rt() const { return (inst >> 16) & 0x1F; }
    uint32_t rd() const { return (inst >> 11) & 0x1F; }
    uint32_t shamt() const { return (inst >> 6) & 0x1F; }
    uint16_t imm16() const { return (uint16_t)(inst & 0xFFFF); }
    uint32_t addr() const { return inst & 0x03FFFFFF; }
};