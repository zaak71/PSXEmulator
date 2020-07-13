#include "CPU.h"
#include "PSX.h"
#include <assert.h>

CPU::CPU(PSX* system) : system(system), next_inst(0) {
    PC = current_PC = 0xBFC00000;
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
    current_PC = PC;
    delay_slot = branch;
    branch = false;
    if (current_PC & 0x03) {
        HandleException(Exceptions::AddrErrorLoad);
        return;
    }
    // Check for any interrupts that need to be handled
    if (COP0.status.current_interrupt_enable) {
        if (COP0.status.interrupt_mask && COP0.cause.interrupt_pending) {
            HandleException(Exceptions::Interrupt);
        }
    }
    uint32_t inst = system->Read32(PC);
    PC = next_PC;
    next_PC += 4;
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
                case 0x02:
                    srl(inst);
                    break;
                case 0x03:
                    sra(inst);
                    break;
                case 0x04:
                    sllv(inst);
                    break;
                case 0x06:
                    srlv(inst);
                    break;
                case 0x07:
                    srav(inst);
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
                case 0x0D:
                    break_(inst);
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
                case 0x18:
                    mult(inst);
                    break;
                case 0x19:
                    multu(inst);
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
        case 0x11:
            HandleCop1(inst);
            break;
        case 0x12:
            HandleCop2(inst);
            break;
        case 0x13:
            HandleCop3(inst);
            break;
        case 0x20:
            lb(inst);
            break;
        case 0x21:
            lh(inst);
            break;
        case 0x22:
            lwl(inst);
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
        case 0x26:
            lwr(inst);
            break;
        case 0x28:
            sb(inst);
            break;
        case 0x29:
            sh(inst);
            break;
        case 0x2A:
            swl(inst);
            break;
        case 0x2B:
            sw(inst);
            break;
        case 0x2E:
            swr(inst);
            break;
        case 0x30:      // LWC0
        case 0x31:      // LWC1
        case 0x33:      // LWC3
        case 0x38:      // SWC0
        case 0x39:      // SWC1
        case 0x3B:      // SWC3
            ExecutePendingLoad();
            HandleException(Exceptions::CpU);
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
    next_PC = PC + imm;
}

void CPU::HandleException(const Exceptions& cause) {
    uint32_t handler = COP0.status.boot_exception_vector ? 0xBFC00180 : 0x80000080;
    uint32_t mode = COP0.status.reg & 0x3F;
    COP0.status.reg &= ~0x3F;
    COP0.status.reg |= ((mode << 2) & 0x3F);
    COP0.cause.reg &= 0x0000FF00;
    COP0.cause.reg |= (((uint32_t)cause) << 2);
    COP0.epc = current_PC;
    
    if (cause == Exceptions::Interrupt) {
        COP0.epc = PC;
    }
    if (delay_slot) {
        COP0.epc -= 4;
        COP0.cause.reg |= (1 << 31);
    }
    else {
        COP0.cause.reg &= ~(1 << 31);
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
    int32_t rs = (int32_t)registers[inst.rs()];
    bool result = rs < 0;
    if (bgez) {
        result = rs >= 0;
    }
    ExecutePendingLoad();
    if (link) {
        registers[31] = next_PC;
    }
    if (result) {
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

void CPU::sllv(const Instruction& inst) {
    uint32_t rt = registers[inst.rt()];
    uint32_t shamt = (registers[inst.rs()] & 0x1F);
    uint32_t result = rt << shamt;
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::srlv(const Instruction& inst) {
    uint32_t result = registers[inst.rt()] >> (registers[inst.rs()] & 0x1F);
    ExecutePendingLoad();
    registers[inst.rd()] = result;
    registers[0] = 0;
}

void CPU::srav(const Instruction& inst) {
    int32_t rt = (int32_t)registers[inst.rt()];
    int32_t shamt = (registers[inst.rs()] & 0x1F);
    int32_t result = rt >> shamt;
    ExecutePendingLoad();
    registers[inst.rd()] = (uint32_t)result;
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
    uint32_t ra = next_PC;
    next_PC = address;
    ExecutePendingLoad();
    registers[inst.rd()] = ra;
    registers[0] = 0;
    branch = true;
}

void CPU::syscall(const Instruction& inst) {
    ExecutePendingLoad();
    HandleException(Exceptions::Syscall);
}

void CPU::break_(const Instruction& inst) {
    ExecutePendingLoad();
    HandleException(Exceptions::Break);
}

void CPU::mthi(const Instruction& inst) {
    uint32_t data = registers[inst.rs()];
    ExecutePendingLoad();
    hi = data;
}


void CPU::mfhi(const Instruction& inst) {
    ExecutePendingLoad();
    registers[inst.rd()] = hi;
    registers[0] = 0;
}

void CPU::mtlo(const Instruction& inst) {
    uint32_t data = registers[inst.rs()];
    ExecutePendingLoad();
    lo = data;
}

void CPU::mflo(const Instruction& inst) {
    ExecutePendingLoad();
    registers[inst.rd()] = lo;
    registers[0] = 0;
}

void CPU::mult(const Instruction& inst) {
    int64_t rs = (int64_t)((int32_t)registers[inst.rs()]);
    int64_t rt = (int64_t)((int32_t)registers[inst.rt()]);
    int64_t result = rs * rt;
    ExecutePendingLoad();
    hi = (uint32_t)((uint64_t)result >> 32);
    lo = (uint32_t)result;
}

void CPU::multu(const Instruction& inst) {
    uint64_t rs = (uint64_t)registers[inst.rs()];
    uint64_t rt = (uint64_t)registers[inst.rt()];
    uint64_t result = rs * rt;
    ExecutePendingLoad();
    hi = (uint32_t)(result >> 32);
    lo = (uint32_t)result;
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
    registers[31] = next_PC;
    next_PC = (PC & 0xF0000000) | (inst.addr() << 2);
    branch = true;
}

void CPU::bne(const Instruction& inst) {
    uint32_t rs_data = registers[inst.rs()];
    uint32_t rt_data = registers[inst.rt()];
    bool branch = rs_data != rt_data;
    ExecutePendingLoad();
    if (branch) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::beq(const Instruction& inst) {
    uint32_t rs_data = registers[inst.rs()];
    uint32_t rt_data = registers[inst.rt()];
    bool branch = rs_data == rt_data;
    ExecutePendingLoad();
    if (branch) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::blez(const Instruction& inst) {
    int32_t rs_data = (int32_t)registers[inst.rs()];
    bool result = rs_data <= 0;
    ExecutePendingLoad();
    if (result) {
        Branch((int32_t)((int16_t)inst.imm16()));
    }
}

void CPU::bgtz(const Instruction& inst) {
    int32_t rs_data = (int32_t)registers[inst.rs()];
    bool result = rs_data > 0;
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
            }
            else {
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
    uint32_t mode = COP0.status.reg & 0x3F;
    COP0.status.reg &= ~(0x3F);
    COP0.status.reg |= (mode >> 2);
}

void CPU::HandleCop1(const Instruction& inst) {
    ExecutePendingLoad();
    HandleException(Exceptions::CpU);
}

void CPU::HandleCop2(const Instruction& inst) {
    switch (inst.rs()) {
        case 0x00:
            mfc2(inst);
            break;
        case 0x02:
            cfc2(inst);
            break;
        case 0x04:
            mtc2(inst);
            break;
        case 0x06:
            ctc2(inst);
            break;
        default:
            printf("Unhandled COP2 instruction: %08x\n", inst.inst);
            assert(false);
            break;
    }
}

void CPU::mfc2(const Instruction& inst) {
    uint32_t rt = inst.rt();
    uint32_t gte_reg = inst.rd();
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = gte.Read(gte_reg);
}

void CPU::cfc2(const Instruction& inst) {
    uint32_t rt = inst.rt();
    uint32_t gte_reg = inst.rd() + 32;
    ExecutePendingLoad();
    pending_reg = rt;
    is_pending_load = true;
    pending_load_data = gte.Read(gte_reg);
}

void CPU::mtc2(const Instruction& inst) {
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    gte.Write(inst.rd(), data);
}

void CPU::ctc2(const Instruction& inst) {
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    gte.Write(inst.rd() + 32, data);
}

void CPU::HandleCop3(const Instruction& inst) {
    ExecutePendingLoad();
    HandleException(Exceptions::CpU);
}

void CPU::lb(const Instruction& inst) {
    if (COP0.status.isolate_cache != 0) {
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
    if (COP0.status.isolate_cache != 0) {
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
    if (COP0.status.isolate_cache != 0) {
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

void CPU::lwl(const Instruction& inst) {
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    if (COP0.status.isolate_cache != 0) {
        return;
    }
    uint32_t rt_data = registers[rt];
    uint32_t aligned_word = system->Read32(address & 0xFFFFFFFC);
    switch (address % 4) {
        case 0:
            pending_load_data = (rt_data & 0x00FFFFFF) | (aligned_word << 24);
            break;
        case 1:
            pending_load_data = (rt_data & 0x0000FFFF) | (aligned_word << 16);
            break;
        case 2:
            pending_load_data = (rt_data & 0x000000FF) | (aligned_word << 8);
            break;
        case 3:
            pending_load_data = (rt_data & 0x00000000) | (aligned_word << 0);
            break;
        default:
            //Should never get here
            assert(false);
    }
    pending_reg = rt;
    is_pending_load = true;
}

void CPU::lbu(const Instruction& inst) {
    if (COP0.status.isolate_cache != 0) {
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
    if (COP0.status.isolate_cache != 0) {
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

void CPU::lwr(const Instruction& inst) {
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t rt = inst.rt();
    ExecutePendingLoad();
    if (COP0.status.isolate_cache != 0) {
        return;
    }
    uint32_t rt_data = registers[rt];
    uint32_t aligned_word = system->Read32(address & 0xFFFFFFFC);
    switch (address % 4) {
    case 0:
        pending_load_data = (rt_data & 0x00000000) | (aligned_word >> 0);
        break;
    case 1:
        pending_load_data = (rt_data & 0xFF000000) | (aligned_word >> 8);
        break;
    case 2:
        pending_load_data = (rt_data & 0xFFFF0000) | (aligned_word >> 16);
        break;
    case 3:
        pending_load_data = (rt_data & 0xFFFFFF00) | (aligned_word >> 24);
        break;
    default:
        //Should never get here
        assert(false);
    }
    pending_reg = rt;
    is_pending_load = true;
}

void CPU::sb(const Instruction& inst) {
    if (COP0.status.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    system->Write8(address, data);
}

void CPU::sh(const Instruction& inst) {
    if (COP0.status.isolate_cache != 0) {
        ExecutePendingLoad();
        return;
    }
    uint32_t rs = registers[inst.rs()];
    uint32_t imm16 = (int32_t)((int16_t)inst.imm16());
    uint32_t address = rs + imm16;
    uint32_t data = registers[inst.rt()];
    ExecutePendingLoad();
    if (address & 0x01) {
        HandleException(Exceptions::AddrErrorStore);
        return;
    }
    system->Write16(address, data);
}

void CPU::swl(const Instruction& inst) {
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t aligned_addr = address & 0xFFFFFFFC;
    uint32_t data = registers[inst.rt()];
    uint32_t current_data = system->Read32(aligned_addr);
    ExecutePendingLoad();
    if (COP0.status.isolate_cache != 0) {
        return;
    }
    switch (address % 4) {
        case 0:
            data = (current_data & 0xFFFFFF00) | (data >> 24);
            break;
        case 1:
            data = (current_data & 0xFFFF0000) | (data >> 16);
            break;
        case 2:
            data = (current_data & 0xFF000000) | (data >> 8);
            break;
        case 3:
            data = (current_data & 0x00000000) | (data >> 0);
            break;
        default:
            assert(false);
            break;
    }
    system->Write32(address, data);
}

void CPU::sw(const Instruction& inst) {
    if (COP0.status.isolate_cache != 0) {
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

void CPU::swr(const Instruction& inst) {
    uint32_t address = registers[inst.rs()] + (int32_t)((int16_t)inst.imm16());
    uint32_t aligned_addr = address & 0xFFFFFFFC;
    uint32_t data = registers[inst.rt()];
    uint32_t current_data = system->Read32(aligned_addr);
    ExecutePendingLoad();
    if (COP0.status.isolate_cache != 0) {
        return;
    }
    switch (address % 4) {
    case 0:
        data = (current_data & 0x00000000) | (data << 0);
        break;
    case 1:
        data = (current_data & 0x000000FF) | (data << 8);
        break;
    case 2:
        data = (current_data & 0x0000FFFF) | (data << 16);
        break;
    case 3:
        data = (current_data & 0x00FFFFFF) | (data << 24);
        break;
    default:
        assert(false);
        break;
    }
    system->Write32(address, data);
}