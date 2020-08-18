#include "Renderer.h"
#include "GPU.h"
#include "GPUCommands.h"
#include "GPUData.h"
#include <algorithm>

Renderer::Renderer(GPU* gpu) {
    this->gpu = gpu;
    
}

int orient2D(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    return ((int)v2.x - v1.x) * ((int)v3.y - v1.y) - ((int)v2.y - v1.y) * ((int)v3.x - v1.x);
}

void Renderer::DrawPolygon(const std::vector<uint32_t>& commands) {
    uint8_t opcode = commands[0] >> 24;
    PolygonArgs args {opcode};
    Vertex vertices[4];
    Color colors[4];
    Texcoord texcoords[4];
    Point points[4];
    // Get data
    if (args.shaded && args.textured) {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[3 * i]);
            vertices[i] = Vertex(commands[3 * i + 1]);
            vertices[i].x += gpu->x_offset;
            vertices[i].y += gpu->y_offset;
            texcoords[i] = Texcoord(commands[3 * i + 2], gpu->GetTexWindowSetting());
            points[i] = {vertices[i], colors[i], texcoords[i]};
        }
    } else if (args.shaded) {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[2 * i]);
            vertices[i] = Vertex(commands[2 * i + 1]);
            vertices[i].x += gpu->x_offset;
            vertices[i].y += gpu->y_offset;
            points[i] = { vertices[i], colors[i], texcoords[0] };
        }
    } else if (args.textured) {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[0]);
            vertices[i] = Vertex(commands[2 * i + 1]);
            vertices[i].x += gpu->x_offset;
            vertices[i].y += gpu->y_offset;
            texcoords[i] = Texcoord(commands[2 * i + 2], gpu->GetTexWindowSetting());
            points[i] = { vertices[i], colors[i], texcoords[i] };
        }
    } else {
        for (int i = 0; i < 3 + (args.four_point); i++) {
            colors[i] = Color(commands[0]);
            vertices[i] = Vertex(commands[i + 1]);
            vertices[i].x += gpu->x_offset;
            vertices[i].y += gpu->y_offset;
            points[i] = { vertices[i], colors[i], texcoords[0] };
        }
    }
    palette = Palette::FromCommand(commands[2]);
    mode.reg = commands[4 + args.shaded] >> 16;

    std::array<Point, 3> point_data = {points[0], points[1], points[2]};
    int area = orient2D(point_data[0].vertex, point_data[1].vertex, point_data[2].vertex);
    if (area < 0) {
        std::swap(point_data[1], point_data[2]);
        area = -area;
    }
    if (area > 0) {
        DrawTriangle(args, point_data, area);
    }
    if (args.four_point) {
        point_data = { points[1], points[2], points[3] };
        int area = orient2D(point_data[0].vertex, point_data[1].vertex, point_data[2].vertex);
        if (area < 0) {
            std::swap(point_data[1], point_data[2]);
            area = -area;
        }
        if (area > 0) {
            DrawTriangle(args, point_data, area);
        }
    }
}

void Renderer::DrawTriangle(const PolygonArgs& args, const std::array<Point, 3>& points,
    const int& area) {
    int min_x = std::min(points[0].vertex.x, std::min(points[1].vertex.x, points[2].vertex.x));
    int min_y = std::min(points[0].vertex.y, std::min(points[1].vertex.y, points[2].vertex.y));
    int max_x = std::max(points[0].vertex.x, std::max(points[1].vertex.x, points[2].vertex.x));
    int max_y = std::max(points[0].vertex.y, std::max(points[1].vertex.y, points[2].vertex.y));

    min_x = std::max((int)gpu->drawing_area_left, std::max(min_x, (int)0));
    min_y = std::max((int)gpu->drawing_area_top, std::max(min_y, (int)0));
    max_x = std::min((int)gpu->drawing_area_right, std::min(max_x, (int)1024));
    max_y = std::min((int)gpu->drawing_area_bottom, std::min(max_y, (int)512));

    Vertex v;
    for (v.y = min_y; v.y < max_y; v.y++) {
        bool entered_row = false;
        for (v.x = min_x; v.x < max_x; v.x++) {
            // calc barycentric coords (not scaled)
            int w0 = orient2D(points[1].vertex, points[2].vertex, v);
            int w1 = orient2D(points[2].vertex, points[0].vertex, v);
            int w2 = orient2D(points[0].vertex, points[1].vertex, v);
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                entered_row = true;
                std::array<int, 3> coords = {w0, w1, w2};
                Color mono = points[0].color;
                Color bg, output;
                if (v.x == 0 && v.y == 0) {
                    printf("here\n");
                }
                bg.raw = gpu->GetVRAMFromPos(v.x, v.y);
                if (!args.shaded && !args.textured) {
                    if (args.semi_trans) {
                        output = Color::Blend(bg, mono, (SemiTransparency)mode.semi_transparency);
                    }
                    gpu->SetVRAMFromPos(v.x, v.y, mono.raw);
                    continue;
                }
                Color interpolated = GetColorFromBarycentricCoords(points, coords);
                if (args.shaded && !args.textured) {
                    if (args.semi_trans) {
                        interpolated = Color::Blend(bg, interpolated, (SemiTransparency)mode.semi_transparency);
                    }
                    gpu->SetVRAMFromPos(v.x, v.y, interpolated.raw);
                    continue;
                }
                // Now all pixels are textured here
                std::array<Texcoord, 3> texcoords = {points[0].texcoord, 
                    points[1].texcoord, points[2].texcoord};
                int x = (texcoords[0].x * coords[0] + texcoords[1].x * coords[1] + texcoords[2].x * coords[2]) 
                    / area;
                int y = (texcoords[0].y * coords[0] + texcoords[1].y * coords[1] + texcoords[2].y * coords[2]) 
                    / area;
                Color tex_color = GetTextureColor(x, y);
                if (tex_color.raw == 0x0000) {
                    continue;
                }
                if (args.raw_tex) {
                    output = tex_color;
                    if (args.semi_trans) {
                        output = Color::Blend(bg, tex_color, (SemiTransparency)mode.semi_transparency);
                    }
                    gpu->SetVRAMFromPos(v.x, v.y, output.raw);
                    continue;
                }
                if (args.shaded) {
                    output = BlendTextureColor(interpolated, tex_color);
                } else {
                    output = BlendTextureColor(mono, tex_color);
                }
                if (args.semi_trans) {
                    if (output.mask) {
                        output = Color::Blend(bg, output, (SemiTransparency)mode.semi_transparency);
                    }
                }
                gpu->SetVRAMFromPos(v.x, v.y, output.raw);
            } else if (entered_row) {
                break;
            }
        }
    }
}

void Renderer::DrawRect(const std::vector<uint32_t>& commands) {
    Color c = Color(commands[0]);
    Vertex source = Vertex(commands[1]);
    source.x += gpu->x_offset;
    source.y += gpu->y_offset;
    Texcoord tex_source;
    RectangleArgs args {commands[0] >> 24};
    if (args.textured) {
        tex_source = Texcoord(commands[2], gpu->GetTexWindowSetting());
        palette = Palette::FromCommand(commands[2]);
    }
    mode = gpu->GetDrawMode();
    uint32_t width = 0, height = 0;
    switch (args.size) {
        case Size::Variable:
            width = commands.back() & 0xFFFFu;
            height = commands.back() >> 16;
            break;
        case Size::Dot:
            width = height = 1;
            break;
        case Size::_8x8:
            width = height = 8;
            break;
        case Size::_16x16:
            width = height = 16;
            break;
        default:    // shouldn't reach here
            break;
    }

    int min_x = std::max((int)gpu->drawing_area_left, std::max((int)source.x, (int)0));
    int min_y = std::max((int)gpu->drawing_area_top, std::max((int)source.y, (int)0));
    int max_x = std::min((int)gpu->drawing_area_right, std::min((int)(source.x + width), (int)1024));
    int max_y = std::min((int)gpu->drawing_area_bottom, std::min((int)(source.y + height), (int)512));

    for (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            Color output;
            if (!args.textured) {
                if (args.semi_trans) {
                    Color bg;
                    bg.raw = gpu->GetVRAMFromPos(x, y);
                    output = Color::Blend(bg, c, (SemiTransparency)mode.semi_transparency);
                }
                gpu->SetVRAMFromPos(x, y, output.raw);
                continue;
            }
            output = GetTextureColor(x - source.x + tex_source.x, y - source.y + tex_source.y);
            if (output.raw == 0x0000) {
                continue;
            }
            if (!args.raw_tex) {
                output = BlendTextureColor(c, output);
            } 
            if (args.semi_trans && (!args.textured || output.mask)) {
                Color bg; 
                bg.raw = gpu->GetVRAMFromPos(x, y);
                output = Color::Blend(bg, output, (SemiTransparency)mode.semi_transparency);
            }
            gpu->SetVRAMFromPos(x, y, output.raw);
        }
    }
}

Color Renderer::BlendTextureColor(const Color& color, const Color& tex_color) const {
    Color c;
    c.r = std::min((color.r * tex_color.r) / 16, 31);
    c.g = std::min((color.g * tex_color.g) / 16, 31);
    c.b = std::min((color.b * tex_color.b) / 16, 31);
    c.mask = tex_color.mask;
    return c;    
}

Color Renderer::GetColorFromBarycentricCoords(const std::array<Point, 3>& points,
    const std::array<int, 3>& coords) {
    float total = (float)(coords[0] + coords[1] + coords[2]);
    int r = points[0].color.r * coords[0] + points[1].color.r * coords[1] + points[2].color.r * coords[2];
    int g = points[0].color.g * coords[0] + points[1].color.g * coords[1] + points[2].color.g * coords[2];
    int b = points[0].color.b * coords[0] + points[1].color.b * coords[1] + points[2].color.b * coords[2];
    r /= total;
    g /= total;
    b /= total;
    return Color(r, g, b);
}

Color Renderer::GetTextureColor(int x, int y) const {
    Color texture_color;
    if (mode.tex_page_colors == TextureDepth::FourBits) {
        uint16_t tex_x = mode.tex_page_x_base * 64 + x / 4;
        uint16_t tex_y = mode.tex_page_y_base * 256 + y;
        uint16_t texel = gpu->GetVRAMFromPos(tex_x, tex_y);
        int index = (texel >> (x % 4) * 4) & 0xFu;
        uint16_t palette_color = gpu->GetVRAMFromPos(palette.x * 16 + index, palette.y);
        texture_color.raw = palette_color;
    } else if (mode.tex_page_colors == TextureDepth::EightBits) {
        uint16_t tex_x = mode.tex_page_x_base * 64 + x / 2;
        uint16_t tex_y = mode.tex_page_y_base * 256 + y;
        uint16_t texel = gpu->GetVRAMFromPos(tex_x, tex_y);
        int index = (texel >> (x % 2) * 8) & 0xFFu;
        uint16_t palette_color = gpu->GetVRAMFromPos(palette.x * 16 + index, palette.y);
        texture_color.raw = palette_color;
    } else if (mode.tex_page_colors == TextureDepth::FifteenBits) {
        uint16_t palette_color = gpu->GetVRAMFromPos(mode.tex_page_x_base * 64 + x, mode.tex_page_y_base * 256 + y);
        texture_color.raw = palette_color;
    }
    return texture_color;
}