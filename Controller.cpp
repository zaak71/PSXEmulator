#include "Controller.h"

uint8_t Controller::Read(uint8_t val) {
    if (read_index == 0) {
        if (val == 0x01) {
            read_index++;
            return 0xFF;
        }
        return 0xFF;
    } else if (read_index == 1) {
        if (val == 0x42) {
            read_index++;
            return 0x41;    // lower bits of controller ID
        }
        return 0xFF;
    } else if (read_index == 2) {
        read_index++;
        return 0x5A;    // high bits of controller ID
    } else if (read_index == 3) {
        read_index++;
        return buttons & 0xFF;
    } else if (read_index == 4) {
        read_index = 0;
        return buttons >> 8;
    }
    return 0xFF;        // invalid read index for digital controller
}