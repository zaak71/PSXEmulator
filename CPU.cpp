#include "CPU.h"
#include "PSX.h"
#include <assert.h>

CPU::CPU(PSX* system) : system(system), next_inst(0) {
    PC = 0xBFC00000;
    next_PC = PC + 4;
    for (uint32_t& i : registers) {
        i = 0xDEADBEEF;
    }
    registers[0] = 0;
    hi = lo = 0;
    pending_reg = pending_load_data = 0;
    is_pending_load = false;
    delay_slot = branch = false;
}

void CPU::RunInstruction() {
    uint32_t pc = PC;
    uint32_t inst = system->Read32(PC);
    PC = next_PC;
    next_PC += 4;
    delay_slot = branch;
    branch = false;
    DecodeAndExecute(inst);
}

void CPU::DecodeAndExecute(uint32_t instruction) {
    current_PC = PC;
    if (current_PC & 0x03) {
        HandleException(Exceptions::AddrErrorLoad);
    }
    Instruction inst(instruction);
    switch (inst.opcode() & 0x3Fu) {
        case 0x00:
            switch (inst.funct() & 0x3Fu) {
                case 0x00:
                    sll(inst);
                    break;
                case 0x02:
                    srl(inst);
                    break;
                case 0x03:
                    sra(inst);
                    break;
                case 0x08:
                    jr(inst);
                    break;
                case 0x09:
                    jalr(inst);
                    break;
                case 0x0C:
                    syscall(inst);
                    break;
                case 0x10:
                    mfhi(inst);
                    break;
                case 0x11:
                    mthi(inst);
                    break;
                case 0x12:
                    mflo(inst);
                    break;
                case 0x13:
                    mtlo(inst);
                    break;
                case 0x1A:
                    div(inst);
                    break;
                case 0x1B:
                    divu(inst);
                    break;
                case 0x20:
                    add(inst);
                    break;
                case 0x21:
                    addu(inst);
                    break;
                case 0x22:
                    sub(inst);
                    break;
                case 0x23:
                    subu(inst);
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
                case 0x2A:
                    slt(inst);
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
        case 0x01:
            branches(inst);
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
        case 0x06:
            blez(inst);
            break;
        case 0x07:
            bgtz(inst);
            break;
        case 0x08:
            addi(inst);
            break;
        case 0x09:
            addiu(inst);
            break;
        case 0x0A:
            slti(inst);
            break;
        case 0x0B:
            sltiu(inst);
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
            HandleCop0(inst);
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
        case 0x24:
            lbu(inst);
            break;
        case 0x25:
            lhu(inst);
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
    branch = true;
    imm = imm << 2;
    uint32_t pc = next_PC;
    pc += imm;
    // Compensate for the hardcoded add of 4
    pc -= 4;
    next_PC = pc;
}

void CPU::HandleCop0(const Instruction& inst) {
    switch (inst.rs() & 0x3F) {
        case 0x00:
            mfc0(inst);
            break;
        case 0x04:
            mtc0(inst);
            break;
        case 0x10:
            if ((inst.funct() & 0x3F) == 0x10) {
                rfe(inst);
            } else {
                printf("Unhandled COP0 instruction: %08x\n", inst.inst);
                assert(false);
            }
            break;
        default:
            printf("Unhandled COP0 instruction: %08x\n", inst.inst);
            assert(false);
            break;
    }
}

void CPU::HandleException(const Exceptions& cause) {
    uint32_t handler = COP0.status_register.status_flags.boot_exception_vector ? 0xBFC00180 : 0x80000080;
    uint32_t mode = COP0.status_register.reg & 0x3F;
    COP0.status_register.reg &= ~0x3F;
    COP0.status_register.reg |= (mode << 2) & 0x3F;
    COP0.cause_register.reg = ((uint32_t)cause << 2);
    COP0.epc = current_PC;

    if (delay_slot) {
        COP0.epc -= 4;
        COP0.cause_register.reg |= (1 << 31);
    }

    PC = handler;
    next_PC = PC + 4;
}

void CPU::sll(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] << inst.shamt();
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::branches(const Instruction& inst) {
    bool bgez = inst.inst >> 16 & 0x01;
    bool link = (inst.inst >> 20 & 0x01) != 0;
    bool result = (int32_t)registers[inst.rs()] < 0;
    if (bgez) {
        result = (int32_t)registers[inst.rs()] >= 0;
    }
    ExecutePendingLoad();
    if (result) {
        if (link) {
            registers[31] = PC;
        }
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::srl(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] >> inst.shamt();
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::sra(const Instruction& inst) {
    uint32_t result = (int32_t)registers[inst.rt()] >> inst.shamt();
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::jr(const Instruction& inst) {
    uint32_t address = registers[inst.rs()];
    ExecutePendingLoad();
    next_PC = address;
    branch = true;
}

void CPU::jalr(const Instruction& inst) {
    uint32_t address = registers[inst.rs()];
    ExecutePendingLoad();
    registers[inst.rd()] = PC;
    next_PC = address;
    branch = true;
}

void CPU::syscall(const Instruction& inst) {
    ExecutePendingLoad();
    HandleException(Exceptions::Syscall);
}

void CPU::mthi(const Instruction& inst) {
    uint32_t data = registers[inst.rs()];
    ExecutePendingLoad();
    hi = data;
}


void CPU::mfhi(const Instruction& inst) {
    ExecutePendingLoad();
    registers[inst.rd()] = hi;
}

void CPU::mtlo(const Instruction& inst) {
    uint32_t data = registers[inst.rs()];
    ExecutePendingLoad();
    lo = data;
}

void CPU::mflo(const Instruction& inst) {
    ExecutePendingLoad();
    registers[inst.rd()] = lo;
}


void CPU::div(const Instruction& inst) {
    int32_t num = (int32_t)registers[inst.rs()];
    int32_t denom = (int32_t)registers[inst.rt()];
    if (denom == 0) {
        hi = (uint32_t)num;
        if (num >= 0) {
            lo = 0xFFFFFFFF;
        } else {
            lo = 1;
        }
    } else if (num == INT32_MIN && denom == -1) {
        lo = 0x80000000;
        hi = 0;
    } else {
        lo = (uint32_t)(num / denom);
        hi = (uint32_t)(num % denom);   
    }
    ExecutePendingLoad();
    registers[0] = 0;
}

void CPU::divu(const Instruction& inst) {
    uint32_t num = registers[inst.rs()];
    uint32_t denom = registers[inst.rt()];
    if (denom == 0) {
        hi = num;
        lo = 0xFFFFFFFF;
    } else {
        lo = (uint32_t)(num / denom);
        hi = (uint32_t)(num % denom);
    }
    ExecutePendingLoad();
    registers[0] = 0;
}

void CPU::add(const Instruction& inst) {
    int32_t rs = (int32_t)registers[inst.rs()];
    int32_t rt = (int32_t)registers[inst.rt()];
    int32_t result = rs + rt;
    ExecutePendingLoad();
    if ((rs >= 0 && rt > INT32_MAX - rs) || (rs < 0 && rt < (INT32_MIN - rs))) {
        HandleException(Exceptions::Overflow);
        return;
    }
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::addu(const Instruction& inst) {
    uint32_t result = registers[inst.rs()] + registers[inst.rt()];
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::sub(const Instruction& inst) {
    int32_t rs = (int32_t)registers[inst.rs()];
    int32_t rt = (int32_t)registers[inst.rt()];
    int32_t result = rs - rt;
    ExecutePendingLoad();
    if ((result < rs) != (rt > 0)) {
        HandleException(Exceptions::Overflow);
        return;
    }
    registers[inst.rd()] = result;
    registers[0] = 0;
}


void CPU::subu(const Instruction& inst) {
    uint32_t result = registers[inst.rs()] - registers[inst.rt()];
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

void CPU::slt(const Instruction& inst) {
    int32_t rs = (int32_t)registers[inst.rs()];
    int32_t rt = (int32_t)registers[inst.rt()];
    bool result = rs < rt;
    ExecutePendingLoad();
    registers[inst.rd()] = result ? 1 : 0;
    registers[0] = 0;
}

void CPU::sltu(const Instruction& inst) {
    bool result = registers[inst.rs()] < registers[inst.rt()];
    ExecutePendingLoad();
    registers[inst.rd()] = result ? 1 : 0;
    registers[0] = 0;
}

void CPU::j(const Instruction& inst) {
    ExecutePendingLoad();
    next_PC = (PC & 0xF0000000) | (inst.addr() << 2);
    branch = true;
}

void CPU::jal(const Instruction& inst) {
    ExecutePendingLoad();
    registers[31] = PC;
    next_PC = (PC & 0xF0000000) | (inst.addr() << 2);
    branch = true;
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

void CPU::blez(const Instruction& inst) {
    bool result = (int32_t)registers[inst.rs()] <= 0;
    ExecutePendingLoad();
    if (result) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::bgtz(const Instruction& inst) {
    bool result = (int32_t)registers[inst.rs()] > 0;
    ExecutePendingLoad();
    if (result) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::addi(const Instruction& inst) {
    int32_t rs = (int32_t)registers[inst.rs()];
    int32_t imm16 = (int32_t)((int16_t)inst.imm16());
    int32_t sum = rs + imm16;
    ExecutePendingLoad();
    if ((rs >= 0 && imm16 > INT32_MAX - rs) || (rs < 0 && imm16 < (INT32_MIN - rs))) {
        HandleException(Exceptions::Overflow);
        return;
    }
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

void CPU::slti(const Instruction& inst) {
    bool result = (int32_t)registers[inst.rs()] < (int32_t)((int16_t)inst.imm16());
    ExecutePendingLoad();
    registers[inst.rt()] = result ? 1 : 0;
    registers[0] = 0;
}

void CPU::sltiu(const Instruction& inst) {
    uint32_t rs = registers[inst.rs()];
    uint32_t imm = (uint32_t)((int32_t)((int16_t)inst.imm16()));
    bool result = rs < imm;
    ExecutePendingLoad();
    registers[inst.rt()] = result ? 1 : 0;
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

void CPU::mfc0(const Instruction& inst) {
    uint32_t rt = inst.rt();
    uint32_t cop0_reg = inst.rd();
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = COP0.Read(cop0_reg);
}

void CPU::mtc0(const Instruction& inst) {
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    COP0.Write(inst.rd(), data);
}

void CPU::rfe(const Instruction& inst) {
    ExecutePendingLoad();
    uint32_t mode = COP0.status_register.reg & 0x3F;
    COP0.status_register.reg &= ~(0x3F);
    COP0.status_register.reg |= (mode >> 2);
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
    if (address & 0x01) {
        HandleException(Exceptions::AddrErrorLoad);
        return;
    }
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
    if (address & 0x03) {
        HandleException(Exceptions::AddrErrorLoad);
        return;
    }
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = system->Read32(address);
}

void CPU::lbu(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = (uint32_t)((uint8_t)system->Read8(address));
}

void CPU::lhu(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    if (address & 0x01) {
        HandleException(Exceptions::AddrErrorLoad);
        return;
    }
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = (uint32_t)((uint16_t)system->Read16(address));
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
    if (address & 0x01) {
        HandleException(Exceptions::AddrErrorStore);
        return;
    }
    system->Write16(address, data);
}

void CPU::sw(const Instruction& inst) {
    if (COP0.status_register.status_flags.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t data = registers[inst.rt()];
    if (address & 0x03) {
        HandleException(Exceptions::AddrErrorStore);
        return;
    }
    ExecutePendingLoad();
    system->Write32(address, data);
}