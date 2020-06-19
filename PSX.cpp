#include "PSX.h"
#include "Constants.h"

#include <cassert>

PSX::PSX() {
    sys_bios = std::make_unique<Bios>(this);
    sys_cpu = std::make_unique<CPU>(this);
    sys_ram = std::make_unique<RAM>();
    sys_spu = std::make_unique<SPU>();
    sys_irq = std::make_unique<IRQ>();
    sys_bios->LoadBios("bios/SCPH1001.BIN");
}

void PSX::Run() {
    sys_cpu->RunInstruction();
}

uint8_t PSX::Read8(uint32_t address) const {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS
        && address + 1 <= RAM_START_ADDRESS + RAM_SIZE) {
        return sys_ram->Read<uint8_t>(address - RAM_START_ADDRESS);
    } else if (address >= BIOS_START_ADDRESS
        && address + 1 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read<uint8_t>(address - BIOS_START_ADDRESS);
    } else if (address >= EXPANSION1_START
        && address + 1 <= EXPANSION1_START + EXPANSION1_SIZE) {
        return 0xFF;
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
    } else if (address >= BIOS_START_ADDRESS
        && address + 2 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read<uint16_t>(address - BIOS_START_ADDRESS);
    } else {
        printf("Unhandled memory access of size 16 at address %08x\n", address);
        assert(false);
        return 0;
    }
}

uint32_t PSX::Read32(uint32_t address) const {
    address = address & region_mask[address >> 29];
    if (address & 0x03) {
        printf("Warning: misaligned memory access of size 32 at address %08x\n", address);
        return 0;
    }

    if (address >= RAM_START_ADDRESS 
        && address + 4 <= RAM_START_ADDRESS + RAM_SIZE) {
        return sys_ram->Read<uint32_t>(address - RAM_START_ADDRESS);
    } else if (address >= BIOS_START_ADDRESS 
        && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read<uint32_t>(address - BIOS_START_ADDRESS);
    } else {
        printf("Unhandled memory access of size 32 at address %08x\n", address);
        assert(false);
        return 0;
    }
}

void PSX::Write32(uint32_t address, const uint32_t data) {
    address = address & region_mask[address >> 29];
    if (address & 0x03) {
        printf("Warning: misaligned memory access of size 32 at address %08x\n", address);
        return;
    }

    if (address >= RAM_START_ADDRESS
        && address + 4 <= RAM_START_ADDRESS + RAM_SIZE) {
        sys_ram->Write(address - RAM_START_ADDRESS, data);
    } else if (address >= BIOS_START_ADDRESS 
        && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        printf("Write to BIOS\n");
    } else if (address >= MEM_CONTROL_1_START 
        && address + 4 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        printf("Write to Memory Control 1\n");
    } else if (address >= MEM_CONTROL_2_START
        && address + 4 <= MEM_CONTROL_2_START + MEM_CONTROL_2_SIZE) {
        printf("Write to Memory Control 2\n");
    } else if (address >= CACHE_CONTROL_START
        && address + 4 <= CACHE_CONTROL_START + CACHE_CONTROL_SIZE) {
        printf("Write to Cache Control\n");
    } else if (address >= IRQ_START
        && address + 4 <= IRQ_START + IRQ_SIZE) {
        sys_irq->Write32(address - IRQ_START, data);
    } else {
        printf("Unhandled write of size 32 at address %08x\n", address);
    }
}

void PSX::Write16(uint32_t address, const uint16_t data) {
    address = address & region_mask[address >> 29];
    if (address & 0x01) {
        printf("Warning: misaligned memory access of size 16 at address %08x\n", address);
        return;
    }

    if (address >= RAM_START_ADDRESS
        && address + 2 <= RAM_START_ADDRESS + RAM_SIZE) {
        sys_ram->Write(address - RAM_START_ADDRESS, data);
    } else if (address >= BIOS_START_ADDRESS 
        && address + 2 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        printf("Write to BIOS\n");
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
    } else {
        printf("Unhandled write of size 16 at address %08x\n", address);
    }
}

void PSX::Write8(uint32_t address, const uint8_t data) {
    address = address & region_mask[address >> 29];

    if (address >= RAM_START_ADDRESS
        && address + 1 <= RAM_START_ADDRESS + RAM_SIZE) {
        sys_ram->Write(address - RAM_START_ADDRESS, data);
    } else if (address >= BIOS_START_ADDRESS 
        && address + 1 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        printf("Write to BIOS\n");
    } else if (address >= MEM_CONTROL_1_START
        && address + 1 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        printf("Write to Memory Control 1\n");
    } else if (address >= MEM_CONTROL_2_START
        && address + 1 <= MEM_CONTROL_2_START + MEM_CONTROL_2_SIZE) {
        printf("Write to Memory Control 2\n");
    } else if (address >= CACHE_CONTROL_START
        && address + 1 <= CACHE_CONTROL_START + CACHE_CONTROL_SIZE) {
        printf("Write to Cache Control\n");
    } else {
        printf("Unhandled write of size 8 at address %08x\n", address);
    }
}