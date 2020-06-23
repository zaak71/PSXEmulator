#pragma once

#include <cstdint>

class GPU {
public:
    uint32_t Read32(uint32_t offset) const;
private:
    union Status {
        uint32_t reg = 0x14802000;
        struct {
            uint32_t tex_page_x_base : 4;		// N*64
            uint32_t tex_page_y_base : 1;		// 0 or 256
            uint32_t semi_transparency : 2;		// 0..3=B/2+F/2, B+F, B-F, B+F/4
            uint32_t tex_page_colors : 2;		// 0..2=4, 8, 15 bits
            uint32_t dither : 1;				// 0=Off/Strip LSBs, 1=Enabled
            uint32_t draw_to_disp_area : 1;		// 0=Prohibited, 1=Allowed
            uint32_t set_mask_bit : 1;			// 0=No, 1=Yes/Mask
            uint32_t draw_pixels : 1;			// 0=Always, 1=Not to Masked Areas
            uint32_t interlace_field : 1;		
            uint32_t reverse_flag : 1;			// 0=Normal, 1=Distorted
            uint32_t tex_disable : 1;			// 0=Normal, 1=Disable Textures
            uint32_t horiz_res_2 : 1;			// 0=horiz_res_1, 1=368
            uint32_t horiz_res_1 : 1;			// 0..3=256, 320, 512, 640
            uint32_t vert_res : 1;				// 0=240, 1=480
            uint32_t video_mode : 1;			// 0=NTSC/60Hz, 1=PAL/50Hz
            uint32_t disp_area_depth : 1;		// 0=15 bit, 1=24
            uint32_t vert_interlace : 1;		// 0=Off, 1=On
            uint32_t disp_enable : 1;			// 0=Enabled, 1=Disabled
            uint32_t irq : 1;					// 0=Off, 1=IRQ
            uint32_t dma : 1;					
            uint32_t ready_for_cmd : 1;			// 0=No, 1=Ready
            uint32_t ready_to_send_vram : 1;	// 0=No, 1=Ready
            uint32_t ready_receive_dma : 1;		// 0=No, 1=Ready
            uint32_t dma_dir : 2;				// 0..3=Off, ?, CPU->GP0, GPUREAD->CPU
            uint32_t draw_even_odd_lines : 1;   // 0=Even/Vblank, 1=Odd
        } flags;
    } GPUSTAT;
};

