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

uint32_t PSX::Read32(const int address) {
    if (address & 0x03) {
        printf("Warning: misaligned memory access of size 32 at address %08x", address);
        return 0;
    }

    // Read from BIOS
    if (address >= BIOS_START_ADDRESS && address + 4 <= BIOS_START_ADDRESS + BIOS_SIZE) {
        return sys_bios->Read32(address - BIOS_START_ADDRESS);
    }
    return 0;
}