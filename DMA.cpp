#include "DMA.h"

#include <cassert>
#include <cstdio>

void DMA::Write32(uint32_t address, uint32_t data) {
    if (address == 0x1F8010F0) {    // Write to control register
        DMA_control_reg.reg = data;
    } else {
        printf("Unhandled Write to DMA at address %08x\n", address);
        assert(false);
    }
}

uint32_t DMA::Read32(uint32_t address) const {
    if (address == 0x1F8010F0) {    // Read control register
        return DMA_control_reg.reg;
    } else {
        printf("Unhandled Read from DMA at address %08x\n", address);
        assert(false);
    }
}
