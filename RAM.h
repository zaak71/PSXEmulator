#pragma once

#include <array>
#include "Constants.h"

class RAM {
public:
   uint32_t Read32(uint32_t address);
   void Write32(uint32_t address, uint32_t data);
private:
    std::array<uint8_t, RAM_SIZE> memory;
};

