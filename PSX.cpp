#include "PSX.h"
#include "Constants.h"

#include <cassert>
#include <fstream>

PSX::PSX() {
    sys_bios = std::make_unique<Bios>(this);
    sys_cpu = std::make_unique<CPU>(this);
    sys_ram = std::make_unique<RAM>();
    sys_spu = std::make_unique<SPU>();
    sys_irq = std::make_unique<IRQ>(sys_cpu.get());
    sys_dma = std::make_unique<DMA>();
    sys_timers = std::make_unique<Timers>();
    sys_gpu = std::make_unique<GPU>();
    sys_cdrom = std::make_unique<cdrom>();
    sys_joypad = std::make_unique<Joypad>();
    sys_scratchpad = std::make_unique<Scratchpad>();

    sys_bios->LoadBios("bios/SCPH1001.BIN");
    sys_dma->Init(sys_ram.get(), this, sys_irq.get(), sys_gpu.get(), sys_cdrom.get(), sys_spu.get());
    sys_gpu->Init(sys_irq.get());
    sys_cdrom->Init(sys_irq.get());
    sys_cpu->AddBreakpoint(0x80030000);
}

void PSX::RunFrame() {
    for (;;) {
        const int cycles = 300;
        if (!sys_cpu->RunInstructions(cycles / 3)) {
            return;
        }
        sys_dma->Cycle();
        sys_cdrom->Cycle();
        sys_timers->Cycle(cycles);
        if (sys_gpu->Cycle(cycles)) {
            return;
        }
    }
}

const GPU::VRAM& PSX::GetVRAM() const {
    return sys_gpu->GetVRAM();
}

void PSX::LoadExe(const std::string& path) {
    std::ifstream exe_file(path, std::ios::binary | std::ios::in | std::ios::ate);
    exe_file.seekg(0, exe_file.end);
    int size = exe_file.tellg();
    exe_file.seekg(0, exe_file.beg);

    exe_data.resize(size);
    if (exe_file.is_open()) {
        exe_file.read((char*)exe_data.data(), size);
    }
    exe_file.close();

    memcpy(&exe, exe_data.data(), sizeof(exe));

    for (int i = 0; i < exe.dest_size; i++) {
        Write8(exe.dest_addr + i, exe_data[0x800 + i]);
    }
}

void PSX::DumpRAM() {
    sys_ram->DumpRAM();
}

void PSX::LoadExeToCPU() {
    sys_cpu->SetPC(exe.initial_pc);
    sys_cpu->SetReg(28, exe.initial_gp);
    if (exe.stack_addr != 0) {
        sys_cpu->SetReg(29, exe.stack_addr + exe.stack_offset);
        sys_cpu->SetReg(30, exe.stack_addr + exe.stack_offset);
    }
}

uint8_t PSX::Read8(uint32_t address) const {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS
        && address + 1 <= RAM_START_ADDRESS + RAM_SIZE) {
        return sys_ram->Read<uint8_t>(address - RAM_START_ADDRESS);
    } else if (address >= SCRATCHPAD_START
        && address + 1 <= SCRATCHPAD_START + SCRATCHPAD_SIZE) {
        return sys_scratchpad->Read<uint8_t>(address - SCRATCHPAD_START);
    } else if (address >= BIOS_START_ADDRESS
        && address + 1 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read<uint8_t>(address - BIOS_START_ADDRESS);
    } else if (address >= EXPANSION1_START
        && address + 1 <= EXPANSION1_START + EXPANSION1_SIZE) {
        return 0;
    } else if (address >= CDROM_START
        && address + 1 <= CDROM_START + CDROM_SIZE) {
        return sys_cdrom->Read8(address - CDROM_START);
    } else if (address >= JOYPAD_START
        && address + 1 <= JOYPAD_START + JOYPAD_SIZE) {
        return sys_joypad->Read8(address - JOYPAD_START);
    } else {
        printf("Unhandled memory read of size 8 at address %08x\n", address);
        assert(false);
        return 0;
    }
}

uint16_t PSX::Read16(uint32_t address) const {
    address = address & region_mask[address >> 29];
    if (address & 0x01) {
        printf("Warning: misaligned memory write of size 16 at address %08x\n", address);
        return 0;
    }

    if (address >= RAM_START_ADDRESS
        && address + 2 <= RAM_START_ADDRESS + RAM_SIZE) {
        return sys_ram->Read<uint16_t>(address - RAM_START_ADDRESS);
    } else if (address >= SCRATCHPAD_START
        && address + 2 <= SCRATCHPAD_START + SCRATCHPAD_SIZE) {
        return sys_scratchpad->Read<uint16_t>(address - SCRATCHPAD_START);
    } else if (address >= BIOS_START_ADDRESS
        && address + 2 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read<uint16_t>(address - BIOS_START_ADDRESS);
    } else if (address >= SPU_START
        && address + 2 <= SPU_START + SPU_SIZE) {
        return sys_spu->Read16(address);
    }  else if (address >= IRQ_START
        && address + 2 <= IRQ_START + IRQ_SIZE) {
        return sys_irq->Read16(address - IRQ_START);
    } else if (address >= JOYPAD_START
        && address + 2 <= JOYPAD_START + JOYPAD_SIZE) {
        return sys_joypad->Read16(address - JOYPAD_START);
    } else if (address >= TIMER_START
        && address + 2 <= TIMER_START + TIMER_SIZE) {
        return sys_timers->Read16(address - TIMER_START);
    } else {
        printf("Unhandled memory access of size 16 at address %08x\n", address);
        assert(false);
        return 0;
    }
}

uint32_t PSX::Read32(uint32_t address) const {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS 
        && address + 4 <= RAM_START_ADDRESS + RAM_SIZE) {
        return sys_ram->Read<uint32_t>(address - RAM_START_ADDRESS);
    } else if (address >= SCRATCHPAD_START
        && address + 4 <= SCRATCHPAD_START + SCRATCHPAD_SIZE) {
        return sys_scratchpad->Read<uint32_t>(address - SCRATCHPAD_START);
    } else if (address >= EXPANSION1_START
        && address + 4 <= EXPANSION1_START + EXPANSION1_SIZE) {
        return 0;
    } else if (address >= BIOS_START_ADDRESS
        && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read<uint32_t>(address - BIOS_START_ADDRESS);
    } else if (address >= IRQ_START
        && address + 4 <= IRQ_START + IRQ_SIZE) {
        return sys_irq->Read32(address - IRQ_START);
    } else if (address >= DMA_START
        && address + 4 <= DMA_START + DMA_SIZE) {
        return sys_dma->Read32(address - DMA_START);
    } else if (address >= GPU_START
        && address + 4 <= GPU_START + GPU_SIZE) {
        printf("Access to GPU at address %08x\n", address);
        return sys_gpu->Read32(address - GPU_START);
    } else if (address >= TIMER_START
        && address + 4 <= TIMER_START + TIMER_SIZE) {
        return sys_timers->Read32(address - TIMER_START);
    } else if (address >= MEM_CONTROL_1_START
        && address + 4 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        return 0;
    } else {
        printf("Unhandled memory access of size 32 at address %08x\n", address);
        assert(false);
        return 0;
    }
}

void PSX::Write32(uint32_t address, const uint32_t data) {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS
        && address + 4 <= RAM_START_ADDRESS + RAM_SIZE) {
        sys_ram->Write<uint32_t>(address - RAM_START_ADDRESS, data);
    } else if (address >= SCRATCHPAD_START
        && address + 4 <= SCRATCHPAD_START + SCRATCHPAD_SIZE) {
        sys_scratchpad->Write<uint32_t>(address - SCRATCHPAD_START, data);
    } else if (address >= BIOS_START_ADDRESS
        && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        sys_bios->Write<uint32_t>(address - BIOS_START_ADDRESS, data);
    } else if (address >= MEM_CONTROL_1_START 
        && address + 4 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        printf("Write to Memory Control 1\n");
    } else if (address >= MEM_CONTROL_2_START
        && address + 4 <= MEM_CONTROL_2_START + MEM_CONTROL_2_SIZE) {
        printf("Write to Memory Control 2\n");
    } else if (address >= CACHE_CONTROL_START
        && address + 4 <= CACHE_CONTROL_START + CACHE_CONTROL_SIZE) {
        printf("Write to Cache Control\n");
    } else if (address >= EXPANSION1_START
        && address + 4 <= EXPANSION1_START + EXPANSION1_SIZE) {
        printf("Write to Expansion 1\n");
    } else if (address >= EXPANSION2_START
        && address + 4 <= EXPANSION2_START + EXPANSION2_SIZE) {
        printf("Write to Expansion 2\n");
    } else if (address >= IRQ_START
        && address + 4 <= IRQ_START + IRQ_SIZE) {
        sys_irq->Write32(address - IRQ_START, data);
    } else if (address >= DMA_START
        && address + 4 <= DMA_START + DMA_SIZE) {
        sys_dma->Write32(address - DMA_START, data);
    } else if (address >= GPU_START
        && address + 4 <= GPU_START + GPU_SIZE) {
        sys_gpu->Write32(address - GPU_START, data);
    } else if (address >= TIMER_START
        && address + 4 <= TIMER_START + TIMER_SIZE) {
        sys_timers->Write32(address - TIMER_START, data);
    } else {
        printf("Unhandled write of size 32 at address %08x, data %08x\n", address, data);
        assert(false);
    }
}

void PSX::Write16(uint32_t address, const uint16_t data) {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS
        && address + 2 <= RAM_START_ADDRESS + RAM_SIZE) {
        sys_ram->Write<uint16_t>(address - RAM_START_ADDRESS, data);
    } else if (address >= SCRATCHPAD_START
        && address + 2 <= SCRATCHPAD_START + SCRATCHPAD_SIZE) {
        sys_scratchpad->Write<uint16_t>(address - SCRATCHPAD_START, data);
    } else if (address >= BIOS_START_ADDRESS
        && address + 2 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        sys_bios->Write<uint16_t>(address - BIOS_START_ADDRESS, data);
    } else if (address >= SPU_START
        && address + 2 <= SPU_START + SPU_SIZE) {
        sys_spu->Write16(address, data);
    } else if (address >= MEM_CONTROL_1_START
        && address + 2 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        printf("Write to Memory Control 1\n");
    } else if (address >= MEM_CONTROL_2_START
        && address + 2 <= MEM_CONTROL_2_START + MEM_CONTROL_2_SIZE) {
        printf("Write to Memory Control 2\n");
    } else if (address >= CACHE_CONTROL_START
        && address + 2 <= CACHE_CONTROL_START + CACHE_CONTROL_SIZE) {
        printf("Write to Cache Control\n");
    } else if (address >= TIMER_START
        && address + 2 <= TIMER_START + TIMER_SIZE) {
        sys_timers->Write16(address - TIMER_START, data);
    } else if (address >= IRQ_START
        && address + 2 <= IRQ_START + IRQ_SIZE) {
        sys_irq->Write16(address - IRQ_START, data);
    } else if (address >= EXPANSION1_START
        && address + 2 <= EXPANSION1_START + EXPANSION1_SIZE) {
        printf("Write to Expansion 2\n");
    } else if (address >= EXPANSION2_START
        && address + 2 <= EXPANSION2_START + EXPANSION2_SIZE) {
        printf("Write to Expansion 2\n");
    } else if (address >= JOYPAD_START 
        && address + 2 <= JOYPAD_START + JOYPAD_SIZE) {
        sys_joypad->Write16(address - JOYPAD_START, data);
    } else {
        printf("Unhandled write of size 16 at address %08x, data %08x\n", address, data);
        assert(false);
    }
}

void PSX::Write8(uint32_t address, const uint8_t data) {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS
        && address + 1 <= RAM_START_ADDRESS + RAM_SIZE) {
        sys_ram->Write<uint8_t>(address - RAM_START_ADDRESS, data);
    } else if (address >= SCRATCHPAD_START
        && address + 1 <= SCRATCHPAD_START + SCRATCHPAD_SIZE) {
        sys_scratchpad->Write<uint8_t>(address - SCRATCHPAD_START, data);
    } else if (address >= BIOS_START_ADDRESS
        && address + 1 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        sys_bios->Write<uint8_t>(address - BIOS_START_ADDRESS, data);
    } else if (address >= MEM_CONTROL_1_START
        && address + 1 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        printf("Write to Memory Control 1\n");
    } else if (address >= MEM_CONTROL_2_START
        && address + 1 <= MEM_CONTROL_2_START + MEM_CONTROL_2_SIZE) {
        printf("Write to Memory Control 2\n");
    } else if (address >= CACHE_CONTROL_START
        && address + 1 <= CACHE_CONTROL_START + CACHE_CONTROL_SIZE) {
        printf("Write to Cache Control\n");
    } else if (address >= EXPANSION1_START
        && address + 1 <= EXPANSION1_START + EXPANSION1_SIZE) {
        printf("Write to Expansion 1\n");
    } else if (address >= EXPANSION2_START
        && address + 1 <= EXPANSION2_START + EXPANSION2_SIZE) {
        printf("Write to Expansion 2\n");
    } else if (address >= CDROM_START
        && address + 1 <= CDROM_START + CDROM_SIZE) {
        sys_cdrom->Write8(address - CDROM_START, data);
    } else if (address >= JOYPAD_START
        && address + 1 <= JOYPAD_START + JOYPAD_SIZE) {
        sys_joypad->Write8(address - JOYPAD_START, data);
    } else {
        printf("Unhandled write of size 8 at address %08x, data %08x\n", address, data);
        assert(false);
    }
}