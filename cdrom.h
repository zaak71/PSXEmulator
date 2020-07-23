#pragma once

#include <cstdint>
#include <deque>
#include "IRQ.h"

class cdrom {
public:
    void Cycle();
    void Init(IRQ* irq);

    void Write8(uint32_t offset, uint8_t data);
    uint8_t Read8(uint32_t offset);
private:
    IRQ* irq;
    
    void ExecuteCommand(uint8_t opcode);
    void TestCommand(uint8_t command);
    uint8_t GetParam();

    union Status {
        uint8_t reg = 0x18;
        struct {
            uint8_t index : 2;
            uint8_t ADPBUSY : 1;            // 0=Empty
            uint8_t param_fifo_empty : 1;   // 1=Empty
            uint8_t param_fifo_full : 1;    // 0=Full
            uint8_t response_fifo_empty : 1;// 0=Empty
            uint8_t data_fifo_empty : 1;    // 0=Empty
            uint8_t cmd_transmission_busy: 1;   // 1=Busy
        };
    } status;

    union StatusCode {
        uint8_t reg = 0x00;                 // sheel is open by default
        struct {
            uint8_t error : 1;
            uint8_t spindle_motor : 1;      // 0=Off/spinning up, 1=On
            uint8_t seek_error : 1;         // 0=OK, 1=Seek Error
            uint8_t id_error : 1;           // 0=OK, 1=GetID denied
            uint8_t shell_open : 1;         // 0=Closed, 1=Is/Was Open
            uint8_t read : 1;
            uint8_t seek : 1;
            uint8_t play : 1;
        };
    } status_code;

    union Mode {
        uint8_t reg = 0;
        struct {
            uint8_t cdda : 1;           // 0=Off, 1=Allow to read CD-DA sectors
            uint8_t auto_pause : 1;     // 0=Off, 1=Auto pause on end of track
            uint8_t report : 1;         // 0=Off, 1=Enable Report-IRQs for audio play
            uint8_t xa_filter : 1;      // 0=Off, 1=Process sectors that match setfilter
            uint8_t ignore_bit : 1;     // 0=Normal, 1=Ignore Sector Size, Setloc pos
            uint8_t sector_size : 1;    // 0=0x800, 1=0x924
            uint8_t xa_adpcm : 1;       // 0=Off, 1=Send sectors to SPU input
            uint8_t speed : 1;          // 0=Normal, 1=Double
        };
    } mode;

    uint8_t irq_enable = 0;

    std::deque<uint8_t> param_fifo = {};
    std::deque<uint8_t> response_fifo = {};
    std::deque<uint8_t> irq_fifo = {};
};

