#pragma once

#include <cstdint>

class DMA {
public:
    void Write32(uint32_t address, uint32_t data);
    uint32_t Read32(uint32_t address) const;
private:
    union ControlReg {
        uint32_t reg = 0x07654321;
        struct {
            uint32_t DMA0_priority : 3;     // 0..7, 0 = Highest, 7 = Lowest
            uint32_t DMA0_enable : 1;       // 0 = Disable, 1 = Enable
            uint32_t DMA1_priority : 3;
            uint32_t DMA1_enable : 1;
            uint32_t DMA2_priority : 3;
            uint32_t DMA2_enable : 1;
            uint32_t DMA3_priority : 3;
            uint32_t DMA3_enable : 1;
            uint32_t DMA4_priority : 3;
            uint32_t DMA4_enable : 1;
            uint32_t DMA5_priority : 3;
            uint32_t DMA5_enable : 1;
            uint32_t DMA6_priority : 3;
            uint32_t DMA6_enable : 1;
        } reg_flags;
    } DMA_control_reg;
};

