#include "Joypad.h"
#include <cassert>
#include <cstdio>

uint8_t Joypad::Read8(uint8_t offset) {
    switch (offset) {
        case 0:     // TODO: implement proper reading from RX FIFO
            if (rx_has_data) {
                rx_has_data = false;
                return rx_data;
            }
            return 0xFF;
        default:
            printf("Unhandled read of size 8 from joypad at offset %01x\n", offset);
            assert(false);
            return 0;
            break;
    }
}

uint16_t Joypad::Read16(uint8_t offset) {
    switch (offset) {
        case 4:
            return 0b101 | ((rx_has_data) << 1);
            // todo: implement proper reading of stat
            break;
        case 0xA:
            return joy_control;
            break;
        case 0xE:
            return joy_baud;
            break;
        default:
            printf("Unhandled read of size 16 from joypad at offset %01x\n", offset);
            assert(false);
            return 0;
            break;
    }
}

void Joypad::Write8(uint8_t offset, uint8_t data) {
    switch (offset) {
        case 0x00:
            Transfer(data);
            break;
        default:
            printf("Unhandled write to joypad at offset %01x, data %04x\n", offset, data);
            assert(false);
            break;
    }
}


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

void Joypad::Transfer(uint8_t data) {
    uint8_t index = (joy_control >> 13) & 1;
    rx_data = controllers[index].Read(data);
    rx_has_data = true;
}