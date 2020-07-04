#include "cdrom.h"

#include <cstdio>
#include <cassert>

void cdrom::Write8(uint32_t offset, uint8_t data) {
    printf("Write of size 8 at CDROM offset %01x, data %02x, index %01x\n", offset, data, status.index);
    if (offset == 0) {
        status.index = data & 0x03;
    } else if (offset == 2 && status.index == 1) {
        irq_enable = data;
    } else if (offset == 3 && status.index == 1) {
        if (data & 0x40) {      // reset the param FIFO
            param_fifo.clear();
            status.param_fifo_empty = 1;
            status.param_fifo_full = 1;
        }
    } else {
        printf("Unhandled CDROM write at offset %01x, data %02x, index %01x\n",
            offset, data, status.index);
        assert(false);
    }
}

uint8_t cdrom::Read8(uint32_t offset) {
    printf("Read of size 8 at CDROM offset %01x, index %01x\n", offset, status.index);
    if (offset == 0) {
        return status.reg;
    } else if (offset == 3 && (status.index == 0 || status.index == 2)) {
        return irq_enable;
    } else if (offset == 3 && (status.index == 1 || status.index == 3)) {
        return 0b11100000;  // TODO: implement reading rest of the bits
    } else {
        printf("Unhandled CDROM read at offset %01x, index %01x\n",
            offset, status.index);
        assert(false);
        return 0;
    }
}