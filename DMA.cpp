#include "DMA.h"

#include <cassert>
#include <cstdio>

void DMA::Write32(uint32_t offset, uint32_t data) {
    uint32_t channel = offset / 0x10;
    if (channel <= 6) {
        switch (offset % 0x10) {
        case 0:
            channels[channel].dma_base_address = data;
            break;
        case 4:
            channels[channel].dma_block_control = data;
            break;
        case 8:
            channels[channel].control.reg = data;
            break;
        default:
            printf("Unhandled Write to DMA at offset %08x\n", offset);
            assert(false);
            break;
        }
    }
    else {
        if (offset == 0x70) {       // Write to control register
            DMA_control_reg.reg = data;
        }
        else if (offset == 0x74) {  // Write to interrupt register
            DMA_interrupt_reg.reg = data;
        }
        else {
            printf("Unhandled Write to DMA at offset %08x\n", offset);
            assert(false);
        }
    }
}

uint32_t DMA::Read32(uint32_t offset) const {
    uint32_t channel = offset / 0x10;
    if (channel <= 6) {
        switch (offset % 0x10) {
            case 0:
                return channels[channel].dma_base_address;
                break;
            case 4:
                return channels[channel].dma_block_control;
                break;
            case 8:
                return channels[channel].control.reg;
                break;
            default:
                printf("Unhandled Read from DMA at offset %08x\n", offset);
                assert(false);
                return 0;
                break;
        }
    }
    else {
        if (offset == 0x70) {    // Read control register
            return DMA_control_reg.reg;
        }
        else if (offset == 0x74) {    // Read interrupt register
            return GetInterruptReg();
        }
        else {
            printf("Unhandled Read from DMA at address %08x\n", offset);
            assert(false);
            return 0;
        }
    }
}

bool DMA::GetMasterFlag() const {
    return DMA_interrupt_reg.flags.force_irq 
        || (DMA_interrupt_reg.flags.irq_master_enable 
        && (DMA_interrupt_reg.flags.irq_flags && DMA_interrupt_reg.flags.irq_enable));
}

uint32_t DMA::GetInterruptReg() const {
    bool master = GetMasterFlag();
    uint32_t reg = DMA_interrupt_reg.reg;
    if (master) {
        reg |= (1 << 31);
    }
    else {
        reg &= (~(1 << 31));
    }
    return reg;
}