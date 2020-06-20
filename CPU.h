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
    uint32_t PC, next_PC, hi, lo;
    uint32_t current_PC;

    uint32_t next_inst;
    bool branch, delay_slot;

    bool is_pending_load;
    uint32_t pending_reg, pending_load_data;

    // Should be called after an instruction is decoded but before writing
    void ExecutePendingLoad();
    void Branch(int imm);

    void HandleCop0(const Instruction& inst);

    enum class Exceptions : uint32_t {
        Int = 0,            // Interrupt (Hardware)
        AddrErrorLoad = 4,  // Address Error (load or inst. fetch)
        AddrErrorStore = 5, // Address Error (store)
        IBE = 6,            // Bus Error (inst. fetch)
        DBE = 7,            // Bus Error (load or store)
        Syscall = 8,        // Syscall Exception
        Bp = 9,             // Breakpoint Exception
        RI = 10,            // Reserved Instruction Exception
        CpU = 11,           // Coprocessor Unimplemented
        Overflow = 12,      // Arithmetic Overflow
        Tr = 13,            // Trap
        FPE = 15            // Floating Point Exception
    };

    void HandleException(const Exceptions& inst);

    void sll(const Instruction& inst);
    void branches(const Instruction& inst);
    void srl(const Instruction& inst);
    void sra(const Instruction& inst);
    void sllv(const Instruction& inst);
    void srlv(const Instruction& inst);
    void srav(const Instruction& inst);
    void jr(const Instruction& inst);
    void jalr(const Instruction& inst);
    void syscall(const Instruction& inst);
    void mflo(const Instruction& inst);
    void mtlo(const Instruction& inst);
    void mfhi(const Instruction& inst);
    void mthi(const Instruction& inst);
    void div(const Instruction& inst);
    void divu(const Instruction& inst);
    void add(const Instruction& inst);
    void addu(const Instruction& inst);
    void sub(const Instruction& inst);
    void subu(const Instruction& inst);
    void and_(const Instruction& inst);
    void or_(const Instruction& inst);
    void xor_(const Instruction& inst);
    void nor(const Instruction& inst);
    void slt(const Instruction& inst);
    void sltu(const Instruction& inst);

    void j(const Instruction& inst);
    void jal(const Instruction& inst);
    void beq(const Instruction& inst);
    void bne(const Instruction& inst);
    void blez(const Instruction& inst);
    void bgtz(const Instruction& inst);
    void addi(const Instruction& inst);
    void addiu(const Instruction& inst);
    void slti(const Instruction& inst);
    void sltiu(const Instruction& inst);
    void andi(const Instruction& inst);
    void ori(const Instruction& inst);
    void xori(const Instruction& inst);
    void lui(const Instruction& inst);
    void mfc0(const Instruction& inst);
    void mtc0(const Instruction& inst);
    void rfe(const Instruction& inst);
    void lb(const Instruction& inst);
    void lh(const Instruction& inst);
    void lw(const Instruction& inst);
    void lbu(const Instruction& inst);
    void lhu(const Instruction& inst);
    void sb(const Instruction& inst);
    void sh(const Instruction& inst);
    void sw(const Instruction& inst);
};

