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
    pending_reg = pending_load_data = 0;
    is_pending_load = false;
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
                case 0x08:
                    jr(inst);
                    break;
                case 0x21:
                    addu(inst);
                    break;
                case 0x24:
                    and_(inst);
                    break;
                case 0x25:
                    or_(inst);
                    break;
                case 0x26:
                    xor_(inst);
                    break;
                case 0x27:
                    nor(inst);
                    break;
                case 0x2B:
                    sltu(inst);
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
        case 0x03:
            jal(inst);
            break;
        case 0x04:
            beq(inst);
            break;
        case 0x05:
            bne(inst);
            break;
        case 0x08:
            addi(inst);
            break;
        case 0x09:
            addiu(inst);
            break;
        case 0x0C:
            andi(inst);
            break;
        case 0x0D:
            ori(inst);
            break;
        case 0x0E:
            xori(inst);
            break;
        case 0x0F:
            lui(inst);
            break;
        case 0x10:
            if (inst.rs() == 0x04) {
                mtc0(inst);
            } else {
                printf("Unhandled COP0 instruction: %08x\n", instruction);
                assert(false);
            }
            break;
        case 0x20:
            lb(inst);
            break;
        case 0x21:
            lh(inst);
            break;
        case 0x23:
            lw(inst);
            break;
        case 0x28:
            sb(inst);
            break;
        case 0x29:
            sh(inst);
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

void CPU::ExecutePendingLoad() {
    if (is_pending_load && pending_reg != 0) {
        registers[pending_reg] = pending_load_data;
    }
    is_pending_load = false;
}

void CPU::Branch(int imm) {
    imm = imm << 2;
    uint32_t pc = PC;
    pc += imm;
    // Compensate for the hardcoded add of 4
    pc -= 4;
    PC = pc;
}

void CPU::HandleCop0(const Instruction& inst) {
    //if (inst.rs())
}

void CPU::sll(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] << inst.shamt();
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::jr(const Instruction& inst) {
    uint32_t address = registers[inst.rs()];
    ExecutePendingLoad();
    PC = address;
}

void CPU::addu(const Instruction& inst) {
    uint32_t result = registers[inst.rs()] + registers[inst.rt()];
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::and_(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] & registers[inst.rs()];
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::or_(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] | registers[inst.rs()];
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::xor_(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] ^ registers[inst.rs()];
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::nor(const Instruction& inst) {
    uint32_t result = ~(registers[inst.rt()] | registers[inst.rs()]);
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::sltu(const Instruction& inst) {
    bool result = registers[inst.rs()] < registers[inst.rt()];
    ExecutePendingLoad();
    registers[inst.rd()] = result ? 1 : 0;
}

void CPU::j(const Instruction& inst) {
    ExecutePendingLoad();
    PC = (PC & 0xF0000000) | (inst.addr() << 2);
}

void CPU::jal(const Instruction& inst) {
    ExecutePendingLoad();
    registers[31] = PC;
    PC = (PC & 0xF0000000) | (inst.addr() << 2);
}

void CPU::bne(const Instruction& inst) {
    bool result = registers[inst.rs()] != registers[inst.rt()];
    ExecutePendingLoad();
    if (result) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::beq(const Instruction& inst) {
    bool result = registers[inst.rs()] == registers[inst.rt()];
    ExecutePendingLoad();
    if (result) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::addi(const Instruction& inst) {
    int32_t rs = (int32_t)registers[inst.rs()];
    int32_t imm16 = (int32_t)((int16_t)inst.imm16());
    int32_t sum = rs + imm16;
    if ((rs >= 0 && imm16 > INT32_MAX - rs) || (rs < 0 && imm16 < (INT32_MIN - rs))) {
        printf("Overflow\n");
        assert(false);
    }
    ExecutePendingLoad();
    registers[inst.rt()] = (uint32_t)sum;
    registers[0] = 0;
}

void CPU::addiu(const Instruction& inst) {
    uint32_t imm = (int32_t)((int16_t)inst.imm16());
    uint32_t result = registers[inst.rs()] + imm;
    ExecutePendingLoad();
    registers[inst.rt()] = result;
    registers[0] = 0;
}

void CPU::andi(const Instruction& inst) {
    uint32_t result = registers[inst.rs()] & (uint32_t)inst.imm16();
    ExecutePendingLoad();
    registers[inst.rt()] = result;
    registers[0] = 0;
}

void CPU::ori(const Instruction& inst) {
    uint32_t result = registers[inst.rs()] | (uint32_t)inst.imm16();
    ExecutePendingLoad();
    registers[inst.rt()] = result;
    registers[0] = 0;
}

void CPU::xori(const Instruction& inst) {
    uint32_t result = registers[inst.rs()] ^ (uint32_t)inst.imm16();
    ExecutePendingLoad();
    registers[inst.rt()] = result;
    registers[0] = 0;
}

void CPU::lui(const Instruction& inst) {
    uint32_t imm = inst.imm16() << 16;
    ExecutePendingLoad();
    registers[inst.rt()] = imm;
    registers[0] = 0;
}

void CPU::mtc0(const Instruction& inst) {
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    COP0.Write(inst.rd(), data);
}

void CPU::lb(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = (int32_t)((int8_t)system->Read8(address));
}

void CPU::lh(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = (int32_t)((int16_t)system->Read16(address));;
}

void CPU::lw(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = system->Read32(address);
}

void CPU::sb(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    system->Write8(address, data);
}

void CPU::sh(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    system->Write16(address, data);
}

void CPU::sw(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    system->Write32(address, data);
}