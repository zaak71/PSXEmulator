#include "RAM.h"

uint32_t RAM::Read32(uint32_t address)
{
    return *(uint32_t*)(memory.data() + address);
}

void RAM::Write32(uint32_t address, uint32_t data) {
    *(uint32_t*)(memory.data() + address) = data;
}
