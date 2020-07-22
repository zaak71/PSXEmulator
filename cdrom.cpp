#include "cdrom.h"

#include <cstdio>
#include <cassert>

void cdrom::Cycle() {
    if (!irq_fifo.empty()) {
        if ((irq_enable & 0x7) && (irq_fifo.front() & 0x7)) {
            irq->TriggerIRQ(2);
        }
    }
}

void cdrom::Init(IRQ* irq) {
    this->irq = irq;
}

void cdrom::Write8(uint32_t offset, uint8_t data) {
    printf("Write of size 8 at CDROM offset %01x, index %01x, data %02x\n", offset, status.index, data);
    if (offset == 0) {
        status.index = data & 0x03;
    } else if (offset == 1 && status.index == 0) {
        ExecuteCommand(data);
    } else if (offset == 2 && status.index == 0) {
        param_fifo.push_back(data);
        status.param_fifo_empty = 0;
        status.param_fifo_full = param_fifo.size() < 16;
    } else if (offset == 2 && status.index == 1) {
        irq_enable = data;
    } else if (offset == 3 && status.index == 1) {
        if (data & 0x40) {      // reset the param FIFO
            param_fifo.clear();
            status.param_fifo_empty = 1;
            status.param_fifo_full = 1;
        }
        if (!irq_fifo.empty()) {
            irq_fifo.pop_front();
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
    } else if (offset == 1) {
        uint8_t response_byte = 0;
        if (!response_fifo.empty()) {
            response_byte = response_fifo.front();
            response_fifo.pop_front();
            if (response_fifo.empty()) {
                status.response_fifo_empty = 0;
            }
        }
        return response_byte;
    } else if (offset == 3 && (status.index == 0 || status.index == 2)) {
        return irq_enable;
    } else if (offset == 3 && (status.index == 1 || status.index == 3)) {
        uint8_t irq = 0;
        if (!irq_fifo.empty()) {
            irq = irq_fifo.front();
            irq_fifo.pop_front();
        }
        return 0b11100000 | (irq & 0x7u);
    } else {
        printf("Unhandled CDROM read at offset %01x, index %01x\n",
            offset, status.index);
        assert(false);
        return 0;
    }
}

void cdrom::ExecuteCommand(uint8_t opcode) {
    switch (opcode) {
        case 0x01:  // GetStat
            response_fifo.push_back(status_code.reg);
            status.response_fifo_empty = 1;
            irq_fifo.push_back(0x3);
            break;
        case 0x0A:  // Init
            irq_fifo.push_back(0x3);
            response_fifo.push_back(status_code.reg);
            status_code.reg &= 0x10;    // reset everything but the shell open
            status_code.spindle_motor = 1;
            mode.reg = 0;
            irq_fifo.push_back(0x2);
            response_fifo.push_back(status_code.reg);
            status.response_fifo_empty = 1;
            break;
        case 0x19:  // Test
            TestCommand(GetParam());
            break;
        default:
            printf("Unhandled CDROM command with opcode %02x\n", opcode);
            assert(false);
            break;
    }
}

void cdrom::TestCommand(uint8_t command) {
    switch (command) {
        case 0x20:
            response_fifo.push_back(0x94);
            response_fifo.push_back(0x09);
            response_fifo.push_back(0x19);
            response_fifo.push_back(0xC0);
            status.response_fifo_empty = 1;
            irq_fifo.push_back(0x3);
            break;
        default:
            printf("Unhandled CDROM test command with opcode %02x\n", command);
            assert(false);
            break;
    }
}

uint8_t cdrom::GetParam() {
    uint8_t param = param_fifo.front();
    param_fifo.pop_front();
    status.param_fifo_empty = param_fifo.empty();
    status.param_fifo_full = 1;
    return param;
}