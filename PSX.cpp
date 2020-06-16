#include "PSX.h"
#include "Constants.h"

PSX::PSX() {
    sys_bios = std::make_unique<Bios>(this);
    sys_cpu = std::make_unique<CPU>(this);
    sys_bios->LoadBios("bios/SCPH1001.BIN");
}

void PSX::Run() {
    sys_cpu->RunInstruction();
}

uint32_t PSX::Read32(uint32_t address) {
    address = address & region_mask[address >> 29];
    if (address & 0x03) {
        printf("Warning: misaligned memory access of size 32 at address %08x\n", address);
        return 0;
    }

    // Read from BIOS
    if (address >= BIOS_START_ADDRESS && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read32(address - BIOS_START_ADDRESS);
    }
    printf("Unhandled memory access of size 32 at address %08x\n", address);
    return 0;
}

void PSX::Write32(uint32_t address, const uint32_t data) {
    address = address & region_mask[address >> 29];
    if (address & 0x03) {
        printf("Warning: misaligned memory access of size 32 at address %08x\n", address);
        return;
    }
    // Write to BIOS
    if (address >= BIOS_START_ADDRESS && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {

    } else if (address >= MEM_CONTROL_1_START 
        && address + 4 <= MEM_CONTROL_1_START + MEM_CONTROL_1_SIZE) {
        printf("Write to Memory Control 1\n");
    } else if (address >= MEM_CONTROL_2_START
        && address + 4 <= MEM_CONTROL_2_START + MEM_CONTROL_2_SIZE) {
        printf("Write to Memory Control 2\n");
    }
    else if (address >= CACHE_CONTROL_START
        && address + 4 <= CACHE_CONTROL_START + CACHE_CONTROL_SIZE) {
        printf("Write to Cache Control\n");
    } else {
        printf("Unhandled write at address %08x\n", address);
    }
}