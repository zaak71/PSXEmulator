#pragma once

#include <cstdint>

class Joypad {
public:
    void Write16(uint16_t offset, uint16_t data);
private:
    uint16_t joy_data = 0;
    uint32_t joy_stat = 0;
    uint16_t joy_mode = 0;
    uint16_t joy_control = 0;
    uint16_t joy_baud = 0;
};

