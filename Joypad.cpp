#include "Joypad.h"
#include <cassert>
#include <cstdio>

void Joypad::Write16(uint16_t offset, uint16_t data) {
    switch (offset) {
        case 0x8:
            joy_mode = data;
            break;
        case 0xA:
            joy_control = data;
            break;
        case 0xE:
            joy_baud = data;
            break;
        default:
            printf("Unhandled write to joypad at offset %01x, data %04x\n", offset, data);
            assert(false);
            break;
    }
}
