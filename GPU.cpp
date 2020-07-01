#include "GPU.h"
#include <cassert>
#include <cstdio>

uint32_t GPU::Read32(uint32_t offset) const {
	switch (offset) {
		case 0:
			return 0x00000000;
			break;
		case 4:
			return GPUSTAT.reg;
			break;
		default:
			printf("Invalid access to GPU at offset %02x\n", offset);
			assert(false);
			return 0;
			break;
	}
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
	switch (opcode) {
		case 0x00:	// nop, do nothing
			break;
		case 0xE1:
			DrawModeSetting(command);
			break;
		case 0xE2:
			TexModeSetting(command);
			break;
		case 0xE3:
			SetDrawingAreaTopLeft(command);
			break;
		case 0xE4:
			SetDrawingAreaBottomRight(command);
			break;
		case 0xE5:
			SetDrawingOffset(command);
			break;
		case 0xE6:
			MaskBitSetting(command);
			break;
		default:
			printf("Unhandled GP0 command: opcode %02x\n", opcode);
			assert(false);
			break;
	}
}

void GPU::GP1Command(uint32_t command) {
	uint32_t opcode = command >> 24;
	switch (opcode) {
		case 0x00:
			ResetGPU();
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

void GPU::DrawModeSetting(uint32_t command) {
	union DrawMode {
		uint32_t reg = 0;
		struct {
			uint32_t tex_page_x_base : 4;		// N*64
			uint32_t tex_page_y_base : 1;		// 0 or 256
			uint32_t semi_transparency : 2;		// 0..3=B/2+F/2, B+F, B-F, B+F/4
			uint32_t tex_page_colors : 2;		// 0..2=4, 8, 15 bits
			uint32_t dither : 1;				// 0=Off/Strip LSBs, 1=Enabled
			uint32_t draw_to_disp_area : 1;		// 0=Prohibited, 1=Allowed
			uint32_t tex_disable : 1;			// 0=Normal, 1=Disable Textures
			uint32_t tex_rect_x_flip : 1;		// BIOS sets this on power up?
			uint32_t tex_rect_y_flip : 1;		// BIOS sets this on GPUSTAT.13?
			uint32_t : 18;
		};
	};
	DrawMode mode {command};
	GPUSTAT.reg &= ~0x3FFu;
	GPUSTAT.reg |= (command & 0x3FFu);
	GPUSTAT.tex_disable = mode.tex_disable;
}

void GPU::TexModeSetting(uint32_t command) {
	tex_window_mask_x = command & 0x1F;
	tex_window_mask_y = (command >> 5) & 0x1F;
	tex_window_offset_x = (command >> 10) & 0x1F;
	tex_window_offset_y = (command >> 15) & 0x1F;
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