#pragma once

#include <array>
#include "Constants.h"

class RAM {
public:
    RAM();
    
    uint32_t Read32(uint32_t address) const;
    void Write32(uint32_t address, uint32_t data);
private:
    std::array<uint8_t, RAM_SIZE> memory;
};

