#pragma once

#include <cstdint>
#include <string>

class DMAChannel {
public:
    bool IsActive() const;
    uint32_t GetTransferLength() const;
    void FinishTransfer();

    enum class SyncMode : uint32_t {
        Manual = 0,             // For OTC and CDROM
        Block = 1,              // For MDEC, SPU, GPU
        LinkedList = 2          // For GPU command lists
    };

    enum class TransferDirection : uint32_t {
        ToRAM = 0,
        FromRAM = 1
    };

    static const char* SyncModeToString(SyncMode sm);
    static const char* TransferDirToString(TransferDirection td);

    uint32_t dma_base_address = 0;      // only 24 bits used
    uint32_t dma_block_control = 0;
    union DMAChannelControl {
        uint32_t reg = 0;
        struct Flags {
            TransferDirection transfer_dir : 1; // 0=To Main RAM, 1=From
            uint32_t mem_addr_step : 1;         // 0=+4, 1=-4
            uint32_t : 6;
            uint32_t chopping_enable : 1;       // 0=Normal, 1=Chopping
            SyncMode sync_mode : 2;
            uint32_t : 5;
            uint32_t chopping_dma_size : 3;     // 1 << N words
            uint32_t : 1;
            uint32_t chopping_cpu_size : 3;     // 1 << N clks
            uint32_t : 1;
            uint32_t enable : 1;                // 0=Stopped/done, 1=Start/Enable/Busy
            uint32_t : 3;
            uint32_t start_trigger : 1;         // 0=Normal, 1=Manual Start
            uint32_t : 3;
        } flags;
    } control;
};

