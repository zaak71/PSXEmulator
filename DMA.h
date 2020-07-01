#pragma once

#include <cstdint>
#include <array>
#include "DMAChannel.h"
#include "RAM.h"
#include "IRQ.h"
#include "GPU.h"

class PSX;

class DMA {
public:
    void Init(RAM* ram, PSX* sys, IRQ* irq, GPU* gpu);
    void Write32(uint32_t offset, uint32_t data);
    uint32_t Read32(uint32_t offset) const;

    void DoTransfer(uint32_t channel);
    void DoManualTransfer(uint32_t channel);
    void DoLinkedTransfer(uint32_t channel);
private:
    enum class Channel : uint32_t {
        MDECIn = 0,
        MDECOut = 1,
        GPU = 2,
        CDROM = 3,
        SPU = 4,
        PIO = 5,
        OTC = 6
    };

    PSX* sys;
    RAM* ram;
    IRQ* irq;
    GPU* gpu;
    std::array<DMAChannel, 7> channels;

    void TriggerInterrupt();
    bool GetMasterFlag() const;
    uint32_t GetInterruptReg() const;

    union ControlReg {
        uint32_t reg = 0x07654321;
        struct {
                                            // 0: MDEC from RAM
            uint32_t DMA0_priority : 3;     // 0..7, 0 = Highest, 7 = Lowest
            uint32_t DMA0_enable : 1;       // 0 = Disable, 1 = Enable
            uint32_t DMA1_priority : 3;     // 1: MDEC to RAM
            uint32_t DMA1_enable : 1;
            uint32_t DMA2_priority : 3;     // 2: GPU
            uint32_t DMA2_enable : 1;
            uint32_t DMA3_priority : 3;     // 3: CDROM
            uint32_t DMA3_enable : 1;
            uint32_t DMA4_priority : 3;     // 4: SPU
            uint32_t DMA4_enable : 1;
            uint32_t DMA5_priority : 3;     // 5: PIO (Expansion)
            uint32_t DMA5_enable : 1;
            uint32_t DMA6_priority : 3;     // 6: OTC (GPU related)
            uint32_t DMA6_enable : 1;
        };
    } DMA_control;

    union InterruptReg {
        uint32_t reg = 0;
        struct {
            uint32_t : 15;
            uint32_t force_irq : 1;         // 0=None, 1=Force Master Flag to 1
            uint32_t irq_enable : 7;        // 0=None, 1=Enable (for DMA0...6)
            uint32_t irq_master_enable : 1; // 0=None, 1=Enable
            uint32_t irq_flags : 7;         // 0=None, 1=IRQ (write 1 to reset)
            uint32_t master_flag : 1;       // 0=None, 1=IRQ
        };
    } DMA_interrupt;
};

