#pragma once

#include <cstdint>

enum class CommandType {
    DrawPolygon,
    DrawLine,
    DrawRect,
    CopyRectangle,
    TransferringCPUtoVRAM,
    Other
};

enum class CopyDirection {
    VRAMtoVRAM,
    CPUtoVRAM,
    VRAMtoCPU,
    None
};

union PolygonArgs {
    uint8_t opcode = 0;
    struct {
        uint8_t raw_tex : 1;        // 0=Tex-blending, 1=Raw-tex
        uint8_t semi_trans : 1;     // 0=Opaque, 1=Semi Transparent
        uint8_t textured : 1;       // 0=No Texture, 1=Textured
        uint8_t four_point : 1;     // 0=3 points, 1=4 points
        uint8_t shaded : 1;         // 0=Monochrome, 1=Shaded
        uint8_t : 3;
    };
    int GetNumArgs() const {
        if (!shaded && !textured) {
            return 4 + four_point;
        } else if (shaded && textured) {
            return 9 + 3 * four_point;
        } else {
            return 6 + textured + 2 * four_point;
        }
    }
};

union LineArgs {
    uint8_t opcode = 0;
    struct {
        uint8_t : 1;
        uint8_t semi_trans : 1; // 0=Opaque, 1=Semi Transparent
        uint8_t : 1;
        uint8_t polyline : 1;   // 0=Line, 1=Polyline
        uint8_t shaded : 1;     // 0=Monochrome, 1=Shaded
        uint8_t : 3;
    };
    int GetNumArgs() const {
        return 3 + shaded;
    }
};

union RectangleArgs {
    uint8_t opcode = 0;
    struct {
        uint8_t raw_tex : 1;    // 0=Texture Blending, 1=Raw Texture
        uint8_t semi_trans : 1; // 0=Opaque, 1=Semi Transparent
        uint8_t textured : 1;   // 0=Monochrome, 1=Textured
        uint8_t size : 2;       // 0=Variable, 1=1x1, 2=8x8, 3=16x16
        uint8_t : 3;
    };
    int GetNumArgs() const {
        int args = 2;
        if (size == 0) {args++;}
        if (textured) {args++;}
        return args;
    }
};

enum class TextureDepth : uint8_t {
    FourBits = 0,
    EightBits = 1,
    FifteenBits = 2,
    Reserved = 3
};

union DrawMode {
    uint32_t reg = 0;
    struct {
        uint32_t tex_page_x_base : 4;		// N*64
        uint32_t tex_page_y_base : 1;		// 0 or 256
        uint32_t semi_transparency : 2;		// 0..3=B/2+F/2, B+F, B-F, B+F/4
        TextureDepth tex_page_colors : 2;		// 0..2=4, 8, 15 bits
        uint32_t dither : 1;				// 0=Off/Strip LSBs, 1=Enabled
        uint32_t draw_to_disp_area : 1;		// 0=Prohibited, 1=Allowed
        uint32_t tex_disable : 1;			// 0=Normal, 1=Disable Textures
        uint32_t tex_rect_x_flip : 1;		// BIOS sets this on power up?
        uint32_t tex_rect_y_flip : 1;		// BIOS sets this on GPUSTAT.13?
        uint32_t : 18;
    };
};

union TextureWindowSetting {
    uint32_t reg = 0;
    struct {
        uint32_t tex_window_mask_x : 5;     // in 8 pixel steps
        uint32_t tex_window_mask_y : 5;     // in 8 pixel steps
        uint32_t tex_window_offset_x : 5;   // in 8 pixel steps
        uint32_t tex_window_offset_y : 5;   // in 8 pixel steps
        uint32_t : 12;
    };
};