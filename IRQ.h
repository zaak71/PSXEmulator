#pragma once

#include <cstdint>

class IRQ {
public:
    void Write32(uint32_t offset, uint32_t data);
    uint32_t Read32(uint32_t offset);
private:
    uint32_t i_stat = 0;
    union Mask {
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
    } i_mask;
};

