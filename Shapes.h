#pragma once

#include <cstdint>
#include <deque>
#include "GPUCommands.h"

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    Color() : r(0), g(0), b(0) {}
    Color(uint32_t color) {         // Constructor from color command
        r = (color >> 0) & 0xFFu;
        g = (color >> 8) & 0xFFu;
        b = (color >> 16) & 0xFFu;
    }
};

struct Vertex {
    uint16_t x;
    uint16_t y;
    Vertex() : x(0), y(0) {}
    Vertex(uint32_t command) {
        x = command & 0x7FF;
        y = (command >> 11) & 0x7FF;
        x = ((int16_t)(x << 5)) >> 5;	// sign extend values
        y = ((int16_t)(y << 5)) >> 5;
    }
};

struct Texcoord {
    int16_t x;
    int16_t y;
    Texcoord() : x(0), y(0) {}
    Texcoord(uint16_t x, uint16_t y) : x(x), y(y) {}
};

struct Polygon {
    Vertex vertices[4];
    Texcoord texcoords[4];
    Color colors[4];
    uint16_t palette = 0;
    uint16_t texpage_x = 0;
    uint16_t texpage_y = 0;

    bool raw_tex;
    bool semi_trans;
    bool textured;
    bool four_point;
    bool shaded;

    void SetAttributes(uint8_t opcode) {
        raw_tex = (opcode >> 0) & 0x1;
        semi_trans = (opcode >> 1) & 0x1;
        textured = (opcode >> 2) & 0x1;
        four_point = (opcode >> 3) & 0x1;
        shaded = (opcode >> 4) & 0x1;
    }

    Polygon(PolygonArgs args, std::deque<uint32_t> commands) {
        SetAttributes(commands[0] >> 24);
        colors[0] = Color(commands[0]);
        vertices[0] = Vertex(commands[1]);
        if (args.shaded && args.textured) {
            colors[1] = Color(commands[3]);
            colors[2] = Color(commands[6]);
            vertices[0] = Vertex(commands[4]);
            vertices[0] = Vertex(commands[7]);
        }
    }
};