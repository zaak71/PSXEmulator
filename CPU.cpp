#include "CPU.h"
#include "PSX.h"
#include <assert.h>

CPU::CPU(PSX* system) : system(system), next_inst(0) {
    PC = 0xBFC00000;
    for (uint32_t& i : registers) {
        i = 0xDEADBEEF;
    }
    registers[0] = 0;
    hi = lo = 0;
}

void CPU::RunInstruction() {
    uint32_t pc = PC;
    uint32_t inst = next_inst;
    next_inst = system->Read32(pc);
    PC += 4;
    DecodeAndExecute(inst);
}

void CPU::DecodeAndExecute(uint32_t instruction) {
    Instruction inst(instruction);
    switch (inst.opcode() & 0x3Fu) {
        case 0x00:
            switch (inst.funct() & 0x3Fu) {
            case 0x00:
                sll(inst);
                break;
            case 0x25:
                or_(inst);
                break;
            default:
                printf("Unhandled Instruction: %08x\n", instruction);
                printf("Unhandled Opcode: %02x\n", inst.opcode());
                printf("Unhandled Function: %02x\n", inst.funct());
                assert(false);
                break;
            }
        break;
        case 0x02:
            j(inst);
            break;
        case 0x09:
            addiu(inst);
            break;
        case 0x0D:
            ori(inst);
            break;
        case 0x0F:
            lui(inst);
            break;
        case 0x2B:
            sw(inst);
            break;
        default:
            printf("Unhandled Instruction: %08x\n", instruction);
            printf("Unhandled Opcode: %02x\n", inst.opcode());
            assert(false);
            break;
    }
}

void CPU::sll(const Instruction& inst) {
    registers[inst.rd()] = registers[inst.rt()] << inst.shamt();
    registers[0] = 0;
}

void CPU::or_(const Instruction& inst) {
    registers[inst.rd()] = registers[inst.rt()] | registers[inst.rs()];
    registers[0] = 0;
}

void CPU::j(const Instruction& inst) {
    PC = (PC & 0xF0000000) | (inst.addr() << 2);
}

void CPU::addiu(const Instruction& inst) {
    registers[inst.rt()] = registers[inst.rs()] + inst.imm();
    registers[0] = 0;
}

void CPU::ori(const Instruction& inst) {
    registers[inst.rt()] = registers[inst.rs()] | inst.imm();
    registers[0] = 0;
}

void CPU::lui(const Instruction& inst) {
    uint32_t imm = inst.imm() << 16;
    registers[inst.rt()] = imm;
    registers[0] = 0;
}

void CPU::sw(const Instruction& inst) {
    system->Write32(registers[inst.rs()] + (int32_t)((int16_t)inst.imm()), registers[inst.rt()]);
}