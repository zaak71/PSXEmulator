#pragma once

#include <cstdint>
#include "Controller.h"

class Joypad {
public:
    uint8_t Read8(uint8_t offset);
    uint16_t Read16(uint8_t offset);
    void Write8(uint8_t offset, uint8_t data);
    void Write16(uint16_t offset, uint16_t data);
private:
    void Transfer(uint8_t data);
    Controller controllers[2];

    bool rx_has_data = false;
    uint8_t rx_data = 0;
    uint16_t joy_data = 0;
    uint32_t joy_stat = 0;
    uint16_t joy_mode = 0;
    uint16_t joy_control = 0;
    uint16_t joy_baud = 0;
};

