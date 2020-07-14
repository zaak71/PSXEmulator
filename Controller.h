#pragma once

#include <cstdint>

class Controller {
public:
    uint8_t Read(uint8_t data);
private:
    uint16_t buttons = 0;
    uint8_t read_index = 0;
};

