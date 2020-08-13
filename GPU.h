#pragma once

#include <cstdint>
#include <deque>
#include <array>

#include "IRQ.h"
#include "GPUCommands.h"
#include "Renderer.h"

#define VRAM_WIDTH      1024
#define VRAM_HEIGHT     512

class GPU {
public:
    void Init(IRQ* irq);
    bool Cycle(int cycles);

    uint32_t Read32(uint32_t offset);
    void Write32(uint32_t offset, uint32_t data);

    void GP0Command(uint32_t command);
    void GP1Command(uint32_t command);

    void DumpVRAM();
    using VRAM = std::array<uint16_t, VRAM_WIDTH * VRAM_HEIGHT>;
    const VRAM& GetVRAM() const { return vram; }
    VRAM& GetVRAM() { return vram; }
    uint16_t GetVRAMFromPos(uint16_t x, uint16_t y) const;
    uint32_t ReadVRAM();
    void SetVRAMFromPos(uint16_t x, uint16_t y, uint16_t data);
    const TextureWindowSetting& GetTexWindowSetting() const {return tex_window_settings;};
    const DrawMode& GetDrawMode() const {return draw_mode;}

    int16_t x_offset = 0;               // -1024...1023
    int16_t y_offset = 0;               // -1024...1023
    uint32_t drawing_area_top = 0;
    uint32_t drawing_area_bottom = 0;
    uint32_t drawing_area_left = 0;
    uint32_t drawing_area_right = 0;
private:
    Renderer renderer = Renderer(this);
    IRQ* irq;
    
    const int kCyclesPerFrame = 33868800 / 60;
    int cycles_ran = 0;
    int frames = 0;
    int gpu_dot = 0;
    int gpu_lines = 0;

    VRAM vram{};
    void MoveVRAMTransferPosition();
    DrawMode draw_mode{};

    uint32_t commands_left = 0;
    std::deque<uint32_t> command_fifo = {};
    CommandType curr_cmd = CommandType::Other;
    CopyDirection copy_dir = CopyDirection::None;

    int GetArgCount(uint8_t opcode) const;

    // GP0 commands
    void FillRectInVRAM();
    void DrawModeSetting(uint32_t command);
    void TexModeSetting(uint32_t command);
    void SetDrawingAreaTopLeft(uint32_t command);
    void SetDrawingAreaBottomRight(uint32_t command);
    void SetDrawingOffset(uint32_t command);
    void MaskBitSetting(uint32_t command);
    void CopyRectCPUtoVRAM(uint32_t data);
    
    // GP1 Commands
    void ResetGPU();
    void SetDMADirection(uint32_t command);
    void SetStartDisplayArea(uint32_t command);
    void SetHorizDisplayRange(uint32_t command);
    void SetVertDisplayRange(uint32_t command);
    void SetDisplayMode(uint32_t command);

    uint32_t disp_start_x = 0;          // (0-1023) halfword addr in VRAM
    uint32_t disp_start_y = 0;          // (0-512) scanline addr in VRAM
    uint32_t horiz_disp_x1 = 0;
    uint32_t horiz_disp_x2 = 0;
    uint32_t vert_disp_y1 = 0;
    uint32_t vert_disp_y2 = 0;
    TextureWindowSetting tex_window_settings;

    uint16_t transfer_start_x = 0;
    uint16_t transfer_start_y = 0;
    uint16_t transfer_width = 0;
    uint16_t transfer_height = 0;
    uint16_t curr_transfer_x = 0;
    uint16_t curr_transfer_y = 0;

    union Status {
        uint32_t reg = 0x1C802000;
        struct {
            uint32_t tex_page_x_base : 4;		// N*64
            uint32_t tex_page_y_base : 1;		// 0 or 256
            SemiTransparency semi_transparency : 2; // 0..3=B/2+F/2, B+F, B-F, B+F/4
            TextureDepth tex_page_colors : 2;		// 0..2=4, 8, 15 bits
            uint32_t dither : 1;				// 0=Off/Strip LSBs, 1=Enabled
            uint32_t draw_to_disp_area : 1;		// 0=Prohibited, 1=Allowed
            uint32_t set_mask_bit : 1;			// 0=No, 1=Yes/Mask
            uint32_t draw_pixels : 1;			// 0=Always, 1=Not to Masked Areas
            uint32_t interlace_field : 1;		
            uint32_t reverse_flag : 1;			// 0=Normal, 1=Distorted
            uint32_t tex_disable : 1;			// 0=Normal, 1=Disable Textures
            uint32_t horiz_res_2 : 1;			// 0=horiz_res_1, 1=368
            uint32_t horiz_res_1 : 2;			// 0..3=256, 320, 512, 640
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
        };
    } GPUSTAT;

    enum class GPUREADMode {
        VRAM,
        GPUInfo
    } read_mode;
    uint32_t read_index = 0;
    uint32_t read_data = 0;
    void GetGPUInfo();
};

