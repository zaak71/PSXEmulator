#include "GPU.h"
#include <cassert>
#include <cstdio>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <vector>

void GPU::Init(IRQ* irq) {
    vram.fill(0);
    this->irq = irq;
}

bool GPU::Cycle(int cycles) {
    cycles_ran += cycles;
    if (cycles_ran >= kCyclesPerFrame) {
        cycles_ran = 0;
        frames++;
        if (frames % 2 == 0) {
            GPUSTAT.draw_even_odd_lines = 0;
        } else {
            GPUSTAT.draw_even_odd_lines = 1;
        }
        irq->TriggerIRQ(0);
        return true;
    }
    return false;
}

uint32_t GPU::Read32(uint32_t offset) {
    switch (offset) {
        case 0:
            return ReadVRAM();
            break;
        case 4:
            // HACK: hardcode bit 19 to zero
            return GPUSTAT.reg & ~(1 << 19);
            break;
        default:
            printf("Invalid access to GPU at offset %02x\n", offset);
            assert(false);
            return 0;
            break;
    }
}

uint32_t GPU::ReadVRAM() {
    uint32_t data = 0;
    data = GetVRAMFromPos(curr_transfer_x, curr_transfer_y);
    MoveVRAMTransferPosition();
    data |= (GetVRAMFromPos(curr_transfer_x, curr_transfer_y) << 16);
    MoveVRAMTransferPosition();
    return data;
}

void GPU::DumpVRAM() {
    std::vector<uint8_t> png(1024 * 512 * 3);
    for (int i = 0; i < vram.size(); i++) {
        png[i * 3 + 0] = ((vram[i] >> 0) & 0x1F) << 3;
        png[i * 3 + 1] = ((vram[i] >> 5) & 0x1F) << 3;
        png[i * 3 + 2] = ((vram[i] >> 10) & 0x1F) << 3;
    }
    stbi_write_png("vram.png", 1024, 512, 3, png.data(), 1024 * 3);
}

void GPU::Write32(uint32_t offset, uint32_t data) {
    switch (offset) {
    case 0:
        printf("GP0 Command (CPU): %08x\n", data);
        GP0Command(data);
        break;
    case 4:
        printf("GP1 Command (CPU): %08x\n", data);
        GP1Command(data);
        break;
    default:
        printf("Invalid access to GPU at offset %02x\n", offset);
        assert(false);
        break;
    }
}

void GPU::GP0Command(uint32_t command) {
    uint32_t opcode = command >> 24;
    
    if (curr_cmd == CommandType::Other) {
        command_fifo.clear();
        command_fifo.push_back(command);
        if (opcode == 0x01 || opcode == 0x00) {
        } else if (opcode == 0x02) {
            curr_cmd = CommandType::FillRectInVRAM;
            commands_left = 2;
        } else if (opcode >= 0x20 && opcode < 0x40) {
            curr_cmd = CommandType::DrawPolygon;
            commands_left = GetArgCount(opcode) - 1;
        } else if (opcode >= 0x60 && opcode < 0x80) {
            curr_cmd = CommandType::DrawRect;
            commands_left = GetArgCount(opcode) - 1;
        } else if (opcode == 0xA0) {
            commands_left = 2;
            curr_cmd = CommandType::CopyRectangle;
            copy_dir = CopyDirection::CPUtoVRAM;
        } else if (opcode == 0xC0) {
            commands_left = 2;
            curr_cmd = CommandType::CopyRectangle;
            copy_dir = CopyDirection::VRAMtoCPU;
        } else if (opcode == 0xE1) {
            DrawModeSetting(command);
        } else if (opcode == 0xE2) {
            TexModeSetting(command);
        } else if (opcode == 0xE3) {
            SetDrawingAreaTopLeft(command);
        } else if (opcode == 0xE4) {
            SetDrawingAreaBottomRight(command);
        } else if (opcode == 0xE5) {
            SetDrawingOffset(command);
        } else if (opcode == 0xE6) {
            MaskBitSetting(command);
        } else {
            printf("Unhandled GP0 command: opcode %02x\n", opcode);
            assert(false);
        }
        return;
    }

    if (curr_cmd == CommandType::TransferringCPUtoVRAM) {
        CopyRectCPUtoVRAM(command);
        return;
    }

    if (commands_left != 0) {
        command_fifo.push_back(command);
        commands_left--;
    }

    if (commands_left != 0) {
        return;
    }

    // Now all the data for drawing or copy params is received
    if (curr_cmd == CommandType::DrawPolygon) {
        printf("Drawing polygon\n");
        uint8_t opcode = command_fifo[0] >> 24;
        std::vector<uint32_t> commands(command_fifo.begin(), command_fifo.end());
        renderer.DrawPolygon(commands);
        curr_cmd = CommandType::Other;
    } else if (curr_cmd == CommandType::DrawRect) {
        printf("Drawing Reactangle\n");
        uint8_t opcode = command_fifo[0] >> 24;
        std::vector<uint32_t> commands(command_fifo.begin(), command_fifo.end());
        renderer.DrawRect(commands);
        curr_cmd = CommandType::Other;
    } else if (curr_cmd == CommandType::CopyRectangle) {
        uint32_t coords = command_fifo[1];
        transfer_start_x = coords & 0x03FFu;
        transfer_start_y = (coords >> 16) & 0x01FFu;
        curr_transfer_x = transfer_start_x;
        curr_transfer_y = transfer_start_y;
        uint32_t size = command_fifo[2];
        transfer_width = size & 0xFFFFu;
        transfer_width = ((transfer_width - 1) & 0x3FFu) + 1;
        transfer_height = (size >> 16) & 0xFFFFu;
        transfer_height = ((transfer_height - 1) & 0x1FFu) + 1;
        size = transfer_width * transfer_height;
        if (size % 2 == 1) {
            size++;
        }
        if (copy_dir == CopyDirection::CPUtoVRAM) {
            printf("Copying Rectangle from CPU to VRAM\n");
            commands_left = size / 2;
            curr_cmd = CommandType::TransferringCPUtoVRAM;
        } else if (copy_dir == CopyDirection::VRAMtoCPU) {
            printf("Copying Rectangle from VRAM to CPU\n");
            curr_cmd = CommandType::Other;
        } else {
            printf("Unhandled Reactangle Copy\n");
            assert(false);
        }
        
    } else if (curr_cmd == CommandType::FillRectInVRAM) {
        FillRectInVRAM();
    } else {
        printf("Unhandled GPU Draw\n");
        assert(false);
    }
    
}

void GPU::GP1Command(uint32_t command) {
    uint32_t opcode = command >> 24;
    switch (opcode) {
        case 0x00:
            ResetGPU();
            break;
        case 0x01:
            command_fifo.clear();
            commands_left = 0;
            curr_cmd = CommandType::Other;
        case 0x02:
            GPUSTAT.irq = 0;
            break;
        case 0x03:
            GPUSTAT.disp_enable = command & 0x01;
            break;
        case 0x04:
            SetDMADirection(command);
            break;
        case 0x05:
            SetStartDisplayArea(command);
            break;
        case 0x06:
            SetHorizDisplayRange(command);
            break;
        case 0x07:
            SetVertDisplayRange(command);
            break;
        case 0x08:
            SetDisplayMode(command);
            break;
        default:
            printf("Unhandled GP1 command: opcode %02x\n", opcode);
            assert(false);
            break;
    }
}

void GPU::FillRectInVRAM() {
    Color color = Color(command_fifo[0]);
    uint32_t x = command_fifo[1] & 0xFFFFu;
    uint32_t y = command_fifo[1] >> 16;
    uint32_t width = command_fifo[2] & 0xFFFFu;
    uint32_t height = command_fifo[2] >> 16;
    width = ((width & 0x3FF) + 0x0F) & (~0x0F);
    height &= 0x1FF;
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            SetVRAMFromPos(i, j, color.raw);
        }
    }
    curr_cmd = CommandType::Other;
}

void GPU::DrawModeSetting(uint32_t command) {
    draw_mode.reg = command;
    GPUSTAT.reg &= ~0x3FFu;
    GPUSTAT.reg |= (command & 0x3FFu);
    GPUSTAT.tex_disable = draw_mode.tex_disable;
}

void GPU::TexModeSetting(uint32_t command) {
    tex_window_settings.reg = command;
}

void GPU::SetDrawingAreaTopLeft(uint32_t command) {
    drawing_area_left = command & 0x3FF;
    drawing_area_top = (command >> 10) & 0x3FF;
}

void GPU::SetDrawingAreaBottomRight(uint32_t command) {
    drawing_area_right = command & 0x3FF;
    drawing_area_bottom = (command >> 10) & 0x3FF;
}

void GPU::SetDrawingOffset(uint32_t command) {
    uint16_t x = command & 0x7FF;
    uint16_t y = (command >> 11) & 0x7FF;
    x_offset = ((int16_t)(x << 5)) >> 5;	// sign extend values
    y_offset = ((int16_t)(y << 5)) >> 5;
}

void GPU::MaskBitSetting(uint32_t command) {
    GPUSTAT.set_mask_bit = command & 0x1;
    GPUSTAT.draw_pixels = (command >> 1) & 0x1;
}

void GPU::CopyRectCPUtoVRAM(uint32_t data) {
    uint16_t data1 = data & 0xFFFFu;
    uint16_t data2 = (data >> 16) & 0xFFFFu;
    SetVRAMFromPos(curr_transfer_x, curr_transfer_y, data1);
    MoveVRAMTransferPosition();
    SetVRAMFromPos(curr_transfer_x, curr_transfer_y, data2);
    MoveVRAMTransferPosition();
    commands_left--;
    if (commands_left == 0) {
        curr_cmd = CommandType::Other;
    }
}

void GPU::ResetGPU() {
    commands_left = 0;
    GPUSTAT.irq = 0;
    GPUSTAT.disp_enable = 1;
    GPUSTAT.dma_dir = 0;
    disp_start_x = 0;
    disp_start_y = 0;
    horiz_disp_x1 = 0x200;
    horiz_disp_x2 = 0xC00;
    vert_disp_y1 = 0x10;
    vert_disp_y2 = 0x100;
    GPUSTAT.horiz_res_1 = 0;
    GPUSTAT.vert_res = 0;
    GPUSTAT.video_mode = 0;
    GPUSTAT.disp_area_depth = 0;
    GPUSTAT.vert_interlace = 0;
    GPUSTAT.horiz_res_2 = 0;
    GPUSTAT.reverse_flag = 0;
    DrawModeSetting(0);
    TexModeSetting(0);
    SetDrawingAreaTopLeft(0);
    SetDrawingAreaBottomRight(0);
    SetDrawingOffset(0);
    MaskBitSetting(0);
}

void GPU::SetDMADirection(uint32_t command) {
    GPUSTAT.dma_dir = command & 0x3;
}

void GPU::SetStartDisplayArea(uint32_t command) {
    disp_start_x = command & 0x3FF;
    disp_start_y = (command >> 10) & 0x1FF;
}

void GPU::SetHorizDisplayRange(uint32_t command) {
    horiz_disp_x1 = command & 0xFFFu;
    horiz_disp_x2 = (command >> 12) & 0xFFFu;
}

void GPU::SetVertDisplayRange(uint32_t command) {
    vert_disp_y1 = command & 0x3FF;
    vert_disp_y2 = (command >> 10) & 0x3FF;
}


void GPU::SetDisplayMode(uint32_t command) {
    GPUSTAT.horiz_res_1 = command & 0x3;
    GPUSTAT.vert_res = (command >> 2) & 0x1;
    GPUSTAT.video_mode = (command >> 3) & 0x1;
    GPUSTAT.disp_area_depth = (command >> 4) & 0x1;
    GPUSTAT.vert_interlace = (command >> 5) & 0x1;
    GPUSTAT.horiz_res_2 = (command >> 6) & 0x1;
    GPUSTAT.reverse_flag = (command >> 7) & 0x1;
}

int GPU::GetArgCount(uint8_t opcode) const {
    if (opcode >= 0x20 && opcode < 0x40) {
        PolygonArgs args{ opcode };
        return args.GetNumArgs();
    } else if (opcode >= 0x40 && opcode < 0x60) {
        LineArgs args{ opcode };
        return args.GetNumArgs();
    } else if (opcode >= 0x60 && opcode < 0x80) {
        RectangleArgs args{ opcode };
        return args.GetNumArgs();
    } else {	// Should not reach here
        return 0;
    }
}

uint16_t GPU::GetVRAMFromPos(uint16_t x, uint16_t y) const {
    return vram[VRAM_WIDTH * y + x];
}

void GPU::SetVRAMFromPos(uint16_t x, uint16_t y, uint16_t data) {
    if (x >= VRAM_WIDTH || y >= VRAM_HEIGHT) {
        return;
    }
    vram[VRAM_WIDTH * y + x] = data;
}

void GPU::MoveVRAMTransferPosition() {
    if (curr_transfer_x == transfer_start_x + transfer_width - 1) {
        curr_transfer_x = transfer_start_x;
        curr_transfer_y++;
    }
    else {
        curr_transfer_x++;
    }
}