#pragma once

#include <memory>
#include <string>

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
#include "MDEC.h"

class PSX {
public:
    PSX();
    void RunFrame();
    const GPU::VRAM& GetVRAM() const;
    void LoadExeToCPU();
    void DumpRAM();

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
    std::unique_ptr<Scratchpad> sys_scratchpad;
    std::unique_ptr<MDEC> sys_mdec;

    const uint32_t region_mask[8] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,     // KUSEG
    0x7FFFFFFF,                                         // KUSEG0
    0x1FFFFFFF,                                         // KUSEG1
    0xFFFFFFFF, 0xFFFFFFFF};                            // KUSEG2

    struct PSEXEHeader {
        char magic[8];  // PS-X EXE
        uint32_t text;
        uint32_t data;

        uint32_t initial_pc;
        uint32_t initial_gp;

        uint32_t dest_addr;
        uint32_t dest_size;

        uint32_t d_addr;
        uint32_t d_size;

        uint32_t memfill_addr;
        uint32_t memfill_size;

        uint32_t stack_addr;
        uint32_t stack_offset;

        uint32_t sp, fp, gp, ret, base;  // reserved, should be 0

        char license[60];
    } exe;

    void LoadExe(const std::string& path);
    std::vector<uint8_t> exe_data{};
};

