#pragma once

#include <vector>
#include <cstdint>
#include <array>
#include "GPUData.h"
#include "GPUCommands.h"

class GPU;

class Renderer {
public:
    Renderer(GPU* gpu);
    void DrawPolygon(const std::vector<uint32_t>& commands);
    void DrawRect(const std::vector<uint32_t>& commands);
private:
    void DrawTriangle(const PolygonArgs& args, const std::array<Point, 3>& points,
        const int& area);
    Color GetColorFromBarycentricCoords(const std::array<Point, 3>& points,
        const std::array<int, 3>& coords);
    Color GetTextureColor(int x, int y) const;
    Color BlendTextureColor(const Color& color, const Color& tex_color) const;

    GPU* gpu;
    Palette palette;
    DrawMode mode;
};

