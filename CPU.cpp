#include "CPU.h"
#include "PSX.h"

CPU::CPU(PSX* system) : system(system) {
    PC = 0xBFC00000;
    for (uint32_t& i : registers) {
        i = 0;
    }
    hi = lo = 0;
}

void CPU::RunInstruction() {
    uint32_t pc = PC;
    uint32_t inst = system->Read32(pc);
    PC += 4;
    DecodeAndExecute(inst);
}

void CPU::DecodeAndExecute(uint32_t instruction) {
    Instruction i;
    i.inst = instruction;
    switch (i.Iinst.opcode) {
        default:
            printf("Unhandled Instruction\n");
    }
}
