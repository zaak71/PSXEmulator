#pragma once

#include <cstdint>
#include "DMAChannel.h"

class DMA {
public:
    void Write32(uint32_t offset, uint32_t data);
    uint32_t Read32(uint32_t offset) const;
private:
    DMAChannel channels[7];

    bool GetMasterFlag() const;
    uint32_t GetInterruptReg() const;

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

    union InterruptReg {
        uint32_t reg = 0;
        struct {
            uint32_t : 15;
            uint32_t force_irq : 1;         // 0=None, 1=Force Master Flag to 1
            uint32_t irq_enable : 7;        // 0=None, 1=Enable (for DMA0...6)
            uint32_t irq_master_enable : 1; // 0=None, 1=Enable
            uint32_t irq_flags : 7;         // 0=None, 1=IRQ (write 1 to reset)
            uint32_t master_flag : 1;       // 0=None, 1=IRQ
        } flags;
    } DMA_interrupt_reg;
};

