#include "DMA.h"
#include "PSX.h"

#include <cassert>
#include <cstdio>

void DMA::Init(RAM* ram, PSX* sys) {
    this->ram = ram;
    this->sys = sys;
}

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
        if (channels[channel].IsActive()) {
            DoTransfer(channel);
        }
    }
    else {
        if (offset == 0x70) {       // Write to control register
            DMA_control_reg.reg = data;
        } else if (offset == 0x74) {  // Write to interrupt register
            DMA_interrupt_reg.reg = data;
        } else {
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

void DMA::DoTransfer(uint32_t channel) {
    assert(channel <= 6);
    DMAChannel& curr_channel = channels[channel];
    DMAChannel::SyncMode sync_mode = curr_channel.control.flags.sync_mode;
    switch (sync_mode) {
        case DMAChannel::SyncMode::Manual:
        case DMAChannel::SyncMode::Sync:
            DoManualTransfer(channel);
            break;
        case DMAChannel::SyncMode::LinkedList:
            DoLinkedTransfer(channel);
            break;
        default:
            const char* mode = DMAChannel::SyncModeToString(sync_mode);
            printf("Attempt to initiate DMA Transfer: mode %s, channel %d\n", mode, channel);
            assert(false);
            break;
        }
}

void DMA::DoManualTransfer(uint32_t channel) {
    DMAChannel& curr_channel = channels[channel];
    uint32_t inc = curr_channel.control.flags.mem_addr_step ? -4 : 4;
    uint32_t size = curr_channel.GetTransferLength();
    uint32_t addr = curr_channel.dma_base_address & 0x00FFFFFF;
    DMAChannel::TransferDirection transfer_direction = curr_channel.control.flags.transfer_dir;
    Channel ch = static_cast<Channel>(channel);

    if (transfer_direction == DMAChannel::TransferDirection::ToRAM) {
        if (ch == Channel::OTC) {
            for (int i = size - 1; i >= 0; i--, addr += inc) {
                uint32_t src = (addr - 4) & 0x00FFFFFC;
                if (i == 0) {
                    src = 0x00FFFFFF;
                }
                sys->Write32(addr, src);
            }
            curr_channel.FinishTransfer();
        } else {
            const char* mode = DMAChannel::TransferDirToString(transfer_direction);
            printf("Attempt to initiate manual DMA Transfer: direction %s, channel %d\n", mode, channel);
            assert(false);
        }
    } else {    // handle transfer from RAM
        if (ch == Channel::GPU) {
            for (uint32_t i = 0; i < size; i++, addr += inc) {
                uint32_t cmd = sys->Read32(addr);
                printf("GP0 command (Manual): %08x\n", cmd);
            }
            curr_channel.FinishTransfer();
        } else {
            const char* mode = DMAChannel::TransferDirToString(transfer_direction);
            printf("Attempt to initiate manual DMA Transfer: direction %s, channel %d\n", mode, channel);
            assert(false);
        }
    }
}

void DMA::DoLinkedTransfer(uint32_t channel) {
    DMAChannel& curr_channel = channels[channel];
    uint32_t inc = curr_channel.control.flags.mem_addr_step ? -4 : 4;
    uint32_t addr = curr_channel.dma_base_address;
    DMAChannel::TransferDirection transfer_direction = curr_channel.control.flags.transfer_dir;
    
    if (transfer_direction == DMAChannel::TransferDirection::FromRAM) {
        Channel ch = static_cast<Channel>(channel);
        if (ch == Channel::GPU) {
            // loop as long as the address is not the end marker
            while (addr != 0x00FFFFFF && addr != 0) {
                uint32_t header = sys->Read32(addr);
                uint32_t size = header >> 24;
                for (uint32_t i = 0; i < size; i++) {
                    addr = (addr + inc) & 0x00FFFFFC;
                    uint32_t cmd = sys->Read32(addr);
                    printf("GPU command (LL): %08x\n", cmd);
                    if (cmd == 0x28030303) {
                        printf("ok\n");
                    }
                }
                addr = header & 0x00FFFFFF;
            }
            curr_channel.dma_base_address = addr;
            curr_channel.FinishTransfer();
        } else {
            const char* mode = DMAChannel::TransferDirToString(transfer_direction);
            printf("Attempt to initiate linked DMA Transfer: direction %s, channel %d\n", mode, channel);
            assert(false);
        }
    } else {
        const char* mode = DMAChannel::TransferDirToString(transfer_direction);
        printf("Attempt to initiate linked DMA Transfer: direction %s, channel %d\n", mode, channel);
        assert(false);
    }
}