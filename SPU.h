#pragma once

#include <cstdint>

class SPU {
public:
    void Write16(uint32_t address, uint16_t data);
private:
    uint16_t main_volume_l = 0;
    uint16_t main_volume_r = 0;
    uint16_t vLOUT = 0;
    uint16_t vROUT = 0;
};

