#include "Renderer.h"
#include "GPU.h"
#include "GPUCommands.h"
#include "GPUData.h"
#include <algorithm>

Renderer::Renderer(GPU* gpu) {
    this->gpu = gpu;
    
}

void Renderer::DrawPolygon(const std::vector<uint32_t>& commands) {
    uint8_t opcode = commands[0] >> 24;
    PolygonArgs args {opcode};
    Vertex vertices[4];
    Color colors[4];
    Texcoord texcoords[4];
    Palette palette;
    // Get data
    if (args.shaded && args.textured) {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[3 * i]);
            vertices[i] = Vertex(commands[3 * i + 1]);
            texcoords[i] = Texcoord(commands[3 * i + 2], gpu->GetTexWindowSetting());
        }
    } else if (args.shaded) {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[2 * i]);
            vertices[i] = Vertex(commands[2 * i + 1]);
        }
    } else if (args.textured) {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[0]);
            vertices[i] = Vertex(commands[2 * i + 1]);
            texcoords[i] = Texcoord(commands[2 * i + 2], gpu->GetTexWindowSetting());
        }
    } else {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[0]);
            vertices[i] = Vertex(commands[i + 1]);
        }
    }
    std::array<Vertex, 3> points = { vertices[0], vertices[1], vertices[2] };
    DrawTriangle(args, points,
        { texcoords[0], texcoords[1], texcoords[2] },
        { colors[0], colors[1], colors[2] });
    if (args.four_point) {
        points = { vertices[1], vertices[2], vertices[3] };
        DrawTriangle(args, points,
            { texcoords[1], texcoords[2], texcoords[3] },
            { colors[1], colors[2], colors[3] });
    }
    gpu->DumpVRAM();
}

int orient2D(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    return ((int)v2.x - v1.x) * ((int)v3.y - v1.y) - ((int)v2.y - v1.y) * ((int)v3.x - v1.x);
}

void Renderer::DrawTriangle(const PolygonArgs& args, std::array<Vertex, 3> points,
    const std::array<Texcoord, 3>& texcoords, const std::array<Color, 3>& colors) {
    int min_x = std::min(points[0].x, std::min(points[1].x, points[2].x));
    int min_y = std::min(points[0].y, std::min(points[1].y, points[2].y));
    int max_x = std::max(points[0].x, std::max(points[1].x, points[2].x));
    int max_y = std::max(points[0].y, std::max(points[1].y, points[2].y));

    int area = orient2D(points[0], points[1], points[2]);
    if (area < 0) {
        std::swap(points[1], points[2]);
    }
    Vertex v;
    for (v.y = min_y; v.y < max_y; v.y++) {
        for (v.x = min_x; v.x < max_x; v.x++) {
            // calc barycentric coords (not scaled)
            int w0 = orient2D(points[1], points[2], v);
            int w1 = orient2D(points[2], points[0], v);
            int w2 = orient2D(points[0], points[1], v);
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                std::array<int, 3> coords = {w0, w1, w2};
                Color c;
                if (!args.shaded && !args.textured) {
                    c = colors[0];
                    gpu->GetVRAM()[v.x + 1024 * v.y] = c.raw;
                } else {
                    c = GetColorFromBarycentricCoords(colors, coords);
                }
                
            }
        }
    }
}

Color Renderer::GetColorFromBarycentricCoords(const std::array<Color, 3>& colors,
    const std::array<int, 3>& coords) {
    float total = (float)(coords[0] + coords[1] + coords[2]);
    int r = colors[0].r * coords[0] + colors[1].r * coords[1] + colors[2].r * coords[2];
    int g = colors[0].g * coords[0] + colors[1].g * coords[1] + colors[2].g * coords[2];
    int b = colors[0].b * coords[0] + colors[1].b * coords[1] + colors[2].b * coords[2];
    r /= total;
    g /= total;
    b /= total;
    return Color(r, g, b);
}