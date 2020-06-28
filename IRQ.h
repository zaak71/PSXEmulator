#pragma once

#include "CPU.h"

#include <cstdint>

class IRQ {
public:
    IRQ(CPU* cpu);

    void TriggerIRQ(int irq);

    void Write32(uint32_t offset, uint32_t data);
    void Write16(uint32_t offset, uint16_t data);
    uint32_t Read32(uint32_t offset) const;
    uint16_t Read16(uint32_t offset) const;
private:
    CPU* cpu;

    union Interrupt {
        uint32_t reg = 0;
        struct Flags {
            uint32_t vblank : 1;
            uint32_t gpu : 1;
            uint32_t cdrom : 1;
            uint32_t dma : 1;
            uint32_t tmr0 : 1;
            uint32_t tmr1 : 1;
            uint32_t tmr2 : 1;
            uint32_t controller_and_mem : 1;
            uint32_t sio : 1;
            uint32_t spu : 1;
            uint32_t lightpen : 1;
            uint32_t : 21;
        } flags;
    } i_mask, i_stat;
};

