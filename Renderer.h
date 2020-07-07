#pragma once

#include <vector>
#include <cstdint>
#include <array>
#include "GPUData.h"

class GPU;

class Renderer {
public:
    Renderer(GPU* gpu);
    void DrawPolygon(const std::vector<uint32_t>& commands);
private:
    void DrawTriangle(const PolygonArgs& args, std::array<Vertex, 3> points,
        const std::array<Texcoord, 3>& texcoords, const std::array<Color, 3>& colors);
    Color GetColorFromBarycentricCoords(const std::array<Color, 3>& colors,
        const std::array<int, 3>& coords);
    //Color GetTextureColor();
    GPU* gpu;
};

