#pragma once

#include <cstdint>
#include "GPUCommands.h"

union Color {
    uint16_t raw = 0;
    struct {
        uint16_t r : 5;
        uint16_t g : 5;
        uint16_t b : 5;
        uint16_t mask : 1;      // 1=Don't overwrite this pixel
    };
    Color() : raw(0) {}
    Color(uint32_t cmd) {
        r = ((cmd >> 0) & 0xFFu) >> 3;
        g = ((cmd >> 8) & 0xFFu) >> 3;
        b = ((cmd >> 16) & 0xFFu) >> 3;
    }
    Color(uint8_t r, uint8_t g, uint8_t b) {
        this->r = r >> 3;
        this->g = g >> 3;
        this->b = b >> 3;
    }
};

struct Vertex {
    int16_t x;
    int16_t y;
    Vertex() : x(0), y(0) {}
    Vertex(uint32_t cmd) {
        x = cmd & 0x7FF;
        y = (cmd >> 16) & 0x7FF;
        x = ((int16_t)(x << 5)) >> 5;	// sign extend values
        y = ((int16_t)(y << 5)) >> 5;
    }
};

struct Texcoord {
    uint8_t x;
    uint8_t y;
    Texcoord() : x(0), y(0) {}
    Texcoord(uint32_t command, const TextureWindowSetting& mode) {
        x = (command >> 0) & 0xFFu;
        y = (command >> 8) & 0xFFu;
        x = (x & ~(mode.tex_window_mask_x * 8)) 
            | ((mode.tex_window_mask_x & mode.tex_window_offset_x) * 8);
        y = (y & ~(mode.tex_window_mask_y * 8)) 
            | ((mode.tex_window_mask_y & mode.tex_window_offset_y) * 8);
    }
};

union Palette {
    uint16_t data = 0;
    struct {
        uint16_t x : 6;     // x * 16
        uint16_t y : 9;
        uint16_t : 1;
    };
    Palette() : data(0) {}
    Palette(uint32_t command) {
        data = command >> 16;
    }
};