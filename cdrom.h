#pragma once

#include <cstdint>
#include <deque>

class cdrom {
public:
    void Write8(uint32_t offset, uint8_t data);
    uint8_t Read8(uint32_t offset);
private:
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

    uint8_t irq_enable = 0;

    std::deque<uint8_t> param_fifo = {};
    std::deque<uint8_t> response_fifo = {};
};

