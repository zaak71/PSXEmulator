#pragma once

#include <memory>

#include "bios.h"
#include "CPU.h"
#include "RAM.h"
#include "SPU.h"
#include "IRQ.h"
#include "DMA.h"
#include "Timers.h"
#include "GPU.h"
#include "cdrom.h"
#include "Joypad.h"

class PSX {
public:
    PSX();
    bool RunStep();
    const GPU::VRAM& GetVRAM() const;

    uint8_t Read8(uint32_t address) const;
    uint16_t Read16(uint32_t address) const;
    uint32_t Read32(uint32_t address) const;
    void Write32(uint32_t address, const uint32_t data);
    void Write16(uint32_t address, const uint16_t data);
    void Write8(uint32_t address, const uint8_t data);
private:
    std::unique_ptr<Bios> sys_bios;
    std::unique_ptr<CPU> sys_cpu;
    std::unique_ptr<RAM> sys_ram;
    std::unique_ptr<SPU> sys_spu;
    std::unique_ptr<IRQ> sys_irq;
    std::unique_ptr<DMA> sys_dma;
    std::unique_ptr<Timers> sys_timers;
    std::unique_ptr<GPU> sys_gpu;
    std::unique_ptr<cdrom> sys_cdrom;
    std::unique_ptr<Joypad> sys_joypad;

    const uint32_t region_mask[8] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,     // KUSEG
    0x7FFFFFFF,                                         // KUSEG0
    0x1FFFFFFF,                                         // KUSEG1
    0xFFFFFFFF, 0xFFFFFFFF};                            // KUSEG2
};

