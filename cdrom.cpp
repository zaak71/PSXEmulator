#include "cdrom.h"

#include <cstdio>
#include <cassert>

void cdrom::Cycle() {
    status.cmd_transmission_busy = 0;
    if (!irq_fifo.empty()) {
        if ((irq_enable & 0x7) && (irq_fifo.front() & 0x7)) {
            irq->TriggerIRQ(2);
        }
    }

    int magic = 1150;
    if (!mode.speed) {
        magic *= 2;
    }
    if (status_code.read) {
        if (--steps_until_read == 0) {
            steps_until_read = magic;

            read_data = game_disk.read(read_sector - 2 * 75);
            read_sector++;

            PushResponse(status_code.reg);
            irq_fifo.push_back(0x1);
        }
    }
}

void cdrom::Init(IRQ* irq) {
    this->irq = irq;
    mm = ss = sect = 0;
    read_sector = seek_sector = 0;
    game_disk.LoadGame("games/castlevania_1.BIN");
}

void cdrom::Write8(uint32_t offset, uint8_t data) {
    printf("Write of size 8 at CDROM offset %01x, index %01x, data %02x\n", offset, status.index, data);
    if (offset == 0) {
        status.index = data & 0x03;
    } else if (offset == 1 && status.index == 0) {
        ExecuteCommand(data);
    } else if (offset == 1 && status.index == 3) {
        vol_right_right = data;
    } else if (offset == 2 && status.index == 0) {
        param_fifo.push_back(data);
        status.param_fifo_empty = 0;
        status.param_fifo_full = param_fifo.size() < 16;
    } else if (offset == 2 && status.index == 1) {
        irq_enable = data;
    } else if (offset == 2 && status.index == 2) {
        vol_left_left = data;
    } else if (offset == 2 && status.index == 3) {
        vol_right_left = data;
    } else if (offset == 3 && status.index == 0) {
        if (data & 0x80 && IsBufferEmpty()) {
            data_buffer = read_data;
            data_buffer_index = 0;
            status.data_fifo_empty = 1;
        } else {
            data_buffer.clear();
            data_buffer_index = 0;
            status.data_fifo_empty = 0;
        }
    } else if (offset == 3 && status.index == 1) {
        if (data & 0x40) {      // reset the param FIFO
            param_fifo.clear();
            status.param_fifo_empty = 1;
            status.param_fifo_full = 1;
        }
        if (!irq_fifo.empty()) {
            irq_fifo.pop_front();
        }
    } else if (offset == 3 && status.index == 2) {
        vol_left_right = data;
    } else if (offset == 3 && status.index == 3) {
        // Apply Volume changes : Ignored for now
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
    irq_fifo.clear();
    response_fifo.clear();
    switch (opcode) {
        case 0x01:  // GetStat
            response_fifo.push_back(status_code.reg);
            status.response_fifo_empty = 1;
            irq_fifo.push_back(0x3);
            break;
        case 0x02: SetLoc(); break;
        case 0x06: ReadN(); break;
        case 0x09: Pause(); break;
        case 0x0A: InitCommand(); break;
        case 0x0B: Mute(); break;
        case 0x0C: Demute(); break;
        case 0x0E: SetMode(); break;
        case 0x13: GetTN(); break;
        case 0x15: SeekL(); break;
        case 0x19: TestCommand(GetParam()); break;
        default:
            printf("Unhandled CDROM command with opcode %02x\n", opcode);
            assert(false);
            break;
    }
    param_fifo.clear();
    status.param_fifo_empty = 1;
    status.param_fifo_full = 1;
    status.cmd_transmission_busy = 1;
    status.ADPBUSY = 0;
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

void cdrom::SetLoc() {
    uint8_t amm = GetParam();
    mm = (amm >> 4) * 10 + (amm & 0xF);
    uint8_t ass = GetParam();
    ss = (ass >> 4) * 10 + (ass & 0xF);
    uint8_t asect = GetParam();
    sect = (asect >> 4) * 10 + (asect & 0xF);
    seek_sector = GetLBA(mm, ss, sect);

    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
}

void cdrom::ReadN() {
    read_sector = seek_sector;

    status_code.reg &= 0x10;
    status_code.spindle_motor = 1;
    status_code.read = 1;

    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
}

void cdrom::Pause() {
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
    
    status_code.reg &= 0x10;
    status_code.spindle_motor = 1;

    irq_fifo.push_back(0x2);
    PushResponse(status_code.reg);
}

void cdrom::SeekL() {
    read_sector = seek_sector;
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);

    status_code.reg &= 0x10;
    status_code.spindle_motor = 1;
    status_code.seek = 1;

    irq_fifo.push_back(0x2);
    PushResponse(status_code.reg);
}

void cdrom::InitCommand() {
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
    status_code.reg &= 0x10;    // reset everything but the shell open
    status_code.spindle_motor = 1;
    mode.reg = 0;
    irq_fifo.push_back(0x2);
    response_fifo.push_back(status_code.reg);
    PushResponse(status_code.reg);
}

void cdrom::Mute() {
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
}

void cdrom::Demute() {
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
}

void cdrom::SetMode() {
    status_code.reg = GetParam();
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
}

void cdrom::GetTN() {
    irq_fifo.push_back(0x3);
    PushResponse(status_code.reg);
    PushResponse(0x1);
    PushResponse(0x3);

}

uint8_t cdrom::GetParam() {
    uint8_t param = param_fifo.front();
    param_fifo.pop_front();
    status.param_fifo_empty = param_fifo.empty();
    status.param_fifo_full = 1;
    return param;
}

void cdrom::PushResponse(uint8_t response) {
    response_fifo.push_back(response);
    status.response_fifo_empty = 1;
}

uint32_t cdrom::GetLBA(uint8_t mm, uint8_t ss, uint8_t sect) const {
    return (mm * 60 * 75) + (ss * 75) + sect;
}

uint8_t cdrom::GetByte() {
    if (data_buffer.empty()) {
        return 0;
    }
    int data_start = 12;
    if (!mode.sector_size) {
        data_start += 12;
    }
    
    if (!mode.sector_size && data_buffer_index >= 0x800) {
        data_buffer_index++;
        return data_buffer[data_start + 0x800 - 8];
    } else if (mode.sector_size && data_buffer_index >= 0x924) {
        data_buffer_index++;
        return data_buffer[data_start + 0x924 - 4];
    }

    uint8_t data = data_buffer[data_start + data_buffer_index++];   // skip the sync

    if (IsBufferEmpty()) {
        status.data_fifo_empty = 0;
    }
    return data;
}

uint32_t cdrom::GetWord() {
    uint32_t word = 0;
    word |= GetByte() << 0;
    word |= GetByte() << 8;
    word |= GetByte() << 16;
    word |= GetByte() << 24;
    return word;
}

bool cdrom::IsBufferEmpty() const {
    return data_buffer.empty() || (!mode.sector_size && data_buffer_index >= 0x800) 
        || (mode.sector_size && data_buffer_index >= 0x924);
}