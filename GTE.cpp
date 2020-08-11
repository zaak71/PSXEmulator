#include "GTE.h"
#include <algorithm>
#include <cassert>

GTE::GTE() : UNR_table(GenerateUNRTable()) {}

uint32_t GetLeadingBitCount(uint32_t num) {
    if (num & 0x80000000) {
        num = ~num; // if the num is negative, flip the bits so same logic is used
    }
    int n = 32;
    unsigned y = num >> 16;
    if (y != 0) {
        n -= 16;
        num = y;
    }
    y = num >> 8;
    if (y != 0) {
        n -= 8;
        num = y;
    }
    y = num >> 4;
    if (y != 0) {
        n -= 4;
        num = y;
    }
    y = num >> 2;
    if (y != 0) {
        n -= 2;
        num = y;
    }
    y = num >> 1;
    if (y != 0) {
        return n - 2;
    }
    return n - num;
}

uint16_t GetLeadingZeroes(uint16_t num) {
    unsigned y = num >> 8;
    int n = 16;
    if (y != 0) {
        n -= 8;
        num = y;
    }
    y = num >> 4;
    if (y != 0) {
        n -= 4;
        num = y;
    }
    y = num >> 2;
    if (y != 0) {
        n -= 2;
        num = y;
    }
    y = num >> 1;
    if (y != 0) {
        return n - 2;
    }
    return n - num;
}

uint32_t GTE::Read(uint32_t reg_num) {
    switch (reg_num) {
        case 0: return (v[0].y << 16) | v[0].x;
        case 1: return v[0].z;
        case 2: return (v[1].y << 16) | v[1].x;
        case 3: return v[1].z;
        case 4: return (v[2].y << 16) | v[2].x;
        case 5: return v[2].z;
        case 6: return (rgbc.a << 24) | (rgbc.b << 16) | (rgbc.g << 8) | (rgbc.r);
        case 7: return average_z;
        case 8: return ir[0];
        case 9: return ir[1];
        case 10: return ir[2];
        case 11: return ir[3];
        case 12: return (sxy[0].y << 16) | (sxy[0].x);
        case 13: return (sxy[1].y << 16) | (sxy[1].x);
        case 14: return (sxy[2].y << 16) | (sxy[2].x);
        case 15: return (sxy[3].y << 16) | (sxy[3].x);
        case 16: return sz[0];
        case 17: return sz[1];
        case 18: return sz[2];
        case 19: return sz[3];
        case 20:
            return (rgb[0].a << 24) | (rgb[0].b << 16) | (rgb[0].g << 8) | (rgb[0].r);
        case 21:
            return (rgb[1].a << 24) | (rgb[1].b << 16) | (rgb[1].g << 8) | (rgb[1].r);
        case 22:
            return (rgb[2].a << 24) | (rgb[2].b << 16) | (rgb[2].g << 8) | (rgb[2].r);
        case 23: return res1;
        case 24: return mac[0];
        case 25: return mac[1];
        case 26: return mac[2];
        case 27: return mac[3];
        case 28:
        case 29: return GetConversionOutput();
        case 30: return lzcs;
        case 31: return lzcr;
        case 32: return (rotation[0][1] << 16) | (rotation[0][0]);
        case 33: return (rotation[1][0] << 16) | (rotation[0][2]);
        case 34: return (rotation[1][2] << 16) | (rotation[1][1]);
        case 35: return (rotation[2][1] << 16) | (rotation[2][0]);
        case 36: return rotation[2][2];
        case 37: return trans_vec.x;
        case 38: return trans_vec.y;
        case 39: return trans_vec.z;
        case 40: return (light_source[0][1] << 16) | (light_source[0][0]);
        case 41: return (light_source[1][0] << 16) | (light_source[0][2]);
        case 42: return (light_source[1][2] << 16) | (light_source[1][1]);
        case 43: return (light_source[2][1] << 16) | (light_source[2][0]);
        case 44: return light_source[2][2];
        case 45: return bg_color.r;
        case 46: return bg_color.g;
        case 47: return bg_color.b;
        case 48: return (light_color[0][1] << 16) | (light_color[0][0]);
        case 49: return (light_color[1][0] << 16) | (light_color[0][2]);
        case 50: return (light_color[1][2] << 16) | (light_color[1][1]);
        case 51: return (light_color[2][1] << 16) | (light_color[2][0]);
        case 52: return light_color[2][2];
        case 53: return far_color.r;
        case 54: return far_color.g;
        case 55: return far_color.b;
        case 56: return offset_x;
        case 57: return offset_y;
        case 58: return (int32_t)((int16_t)h);
        case 59: return dqa;
        case 60: return dqb;
        case 61: return zsf3;
        case 62: return zsf4;
        case 63: return flag.read();
        default: return 0; break;
    }
}

void GTE::Write(uint32_t reg_num, uint32_t data) {
    switch (reg_num) {
        case 0:
            v[0].x = data & 0xFFFFu;
            v[0].y = data >> 16;
            break;
        case 1:
            v[0].z = data;
            break;
        case 2:
            v[1].x = data & 0xFFFFu;
            v[1].y = data >> 16;
            break;
        case 3:
            v[1].z = data;
            break;
        case 4:
            v[2].x = data & 0xFFFFu;
            v[2].y = data >> 16;
            break;
        case 5:
            v[2].z = data;
            break;
        case 6:
            rgbc.r = (data >> 0) & 0xFFu;
            rgbc.g = (data >> 8) & 0xFFu;
            rgbc.b = (data >> 16) & 0xFFu;
            rgbc.a = (data >> 24) & 0xFFu;
            break;
        case 7: average_z = data; break;
        case 8: ir[0] = data; break;
        case 9: ir[1] = data; break;
        case 10: ir[2] = data; break;
        case 11: ir[3] = data; break;
        case 12: 
            sxy[0].x = data & 0xFFFFu;
            sxy[0].y = data >> 16;
            break;
        case 13:
            sxy[1].x = data & 0xFFFFu;
            sxy[1].y = data >> 16;
            break;
        case 14:
            sxy[2].x = data & 0xFFFFu;
            sxy[2].y = data >> 16;
            break;
        case 15:
            sxy[0].x = sxy[1].x;
            sxy[0].y = sxy[1].y;
            sxy[1].x = sxy[2].x;
            sxy[1].y = sxy[2].y;
            sxy[2].x = data & 0xFFFFu;
            sxy[2].y = data >> 16;
            break;
        case 16: sz[0] = data; break;
        case 17: sz[1] = data; break;
        case 18: sz[2] = data; break;
        case 19: sz[3] = data; break;
        case 20:
            rgb[0].r = (data >> 0) & 0xFFu;
            rgb[0].g = (data >> 8) & 0xFFu;
            rgb[0].b = (data >> 16) & 0xFFu;
            rgb[0].a = (data >> 24) & 0xFFu;
            break;
        case 21:
            rgb[1].r = (data >> 0) & 0xFFu;
            rgb[1].g = (data >> 8) & 0xFFu;
            rgb[1].b = (data >> 16) & 0xFFu;
            rgb[1].a = (data >> 24) & 0xFFu;
            break;
        case 22:
            rgb[2].r = (data >> 0) & 0xFFu;
            rgb[2].g = (data >> 8) & 0xFFu;
            rgb[2].b = (data >> 16) & 0xFFu;
            rgb[2].a = (data >> 24) & 0xFFu;
            break;
        case 23: res1 = data; break;
        case 24: mac[0] = data; break;
        case 25: mac[1] = data; break;
        case 26: mac[2] = data; break;
        case 27: mac[3] = data; break;
        case 28:
            irgb = data & 0x7FFFu;
            ir[1] = ((data >> 0) & 0x1F) * 0x80;
            ir[2] = ((data >> 5) & 0x1F) * 0x80;
            ir[3] = ((data >> 10) & 0x1F) * 0x80;
            break;
        case 29: break;
        case 30:
            lzcs = data;
            lzcr = GetLeadingBitCount(data);
            break;
        case 32:
            rotation[0][0] = data & 0xFFFFu;
            rotation[0][1] = data >> 16;
            break;
        case 33:
            rotation[0][2] = data & 0xFFFFu;
            rotation[1][0] = data >> 16;
            break;
        case 34:
            rotation[1][1] = data & 0xFFFFu;
            rotation[1][2] = data >> 16;
            break;
        case 35:
            rotation[2][0] = data & 0xFFFFu;
            rotation[2][1] = data >> 16;
            break;
        case 36: rotation[2][2] = data & 0xFFFFu; break;
        case 37: trans_vec.x = data; break;
        case 38: trans_vec.y = data; break;
        case 39: trans_vec.z = data; break;
        case 40:
            light_source[0][0] = data & 0xFFFFu;
            light_source[0][1] = data >> 16;
            break;
        case 41:
            light_source[0][2] = data & 0xFFFFu;
            light_source[1][0] = data >> 16;
            break;
        case 42:
            light_source[1][1] = data & 0xFFFFu;
            light_source[1][2] = data >> 16;
            break;
        case 43:
            light_source[2][0] = data & 0xFFFFu;
            light_source[2][1] = data >> 16;
            break;
        case 44: light_source[2][2] = data & 0xFFFFu; break;
        case 45: bg_color.r = data; break;
        case 46: bg_color.g = data; break;
        case 47: bg_color.b = data; break;
        case 48:
            light_color[0][0] = data & 0xFFFFu;
            light_color[0][1] = data >> 16;
            break;
        case 49:
            light_color[0][2] = data & 0xFFFFu;
            light_color[1][0] = data >> 16;
            break;
        case 50:
            light_color[1][1] = data & 0xFFFFu;
            light_color[1][2] = data >> 16;
            break;
        case 51:
            light_color[2][0] = data & 0xFFFFu;
            light_color[2][1] = data >> 16;
            break;
        case 52: light_color[2][2] = data & 0xFFFFu; break;
        case 53: far_color.r = data & 0xFFFFu; break;
        case 54: far_color.g = data & 0xFFFFu; break;
        case 55: far_color.b = data & 0xFFFFu; break;
        case 56: offset_x = data; break;
        case 57: offset_y = data; break;
        case 58: h = data; break;
        case 59: dqa = data; break;
        case 60: dqb = data; break;
        case 61: zsf3 = data; break;
        case 62: zsf4 = data; break;
        case 63: flag.reg = data & 0x7FFFF000; break;
        default:
            break;
    }
}

uint32_t GTE::GetConversionOutput() {
    uint32_t output = 0;
    output |= glm::clamp(ir[1] / 0x80, 0, 0x1F);
    output |= (glm::clamp(ir[2] / 0x80, 0, 0x1F)) << 5;
    output |= (glm::clamp(ir[3] / 0x80, 0, 0x1F)) << 10;
    return output;
}

void GTE::ExecuteGTECommand(uint32_t inst) {
    flag.reg = 0;
    current_inst = GTEInstruction{inst};
    switch (current_inst.cmd_num) {
        case 0x01: RTPS(0); break;
        case 0x06: NCLIP(); break;
        case 0x12: MVMVA(); break;
        case 0x1B: NCCS(0); break;
        case 0x30: RTPT(); break;
        case 0x3F: NCCT(); break;
        default:
            printf("Unhandled GTE command: %08x\n GTE opcode: %02x\n", inst, current_inst.cmd_num);
            assert(false);
            break;
    }
}

void GTE::MVMVA() {
    Matrix mx{};
    if (current_inst.mult_mat == 0) {
        mx = rotation;
    } else if (current_inst.mult_mat == 1) {
        mx = light_source;
    } else if (current_inst.mult_mat == 2) {
        mx = light_color;
    } else if (current_inst.mult_mat == 3) {
        mx[0][0] = 0x60;
        mx[0][1] = -0x60;
        mx[0][2] = ir[0];
        mx[1][0] = mx[1][1] = mx[1][2] = rotation[0][2];
        mx[2][0] = mx[2][1] = mx[2][2] = rotation[1][1];
    }

    Vec3 vx{};
    if (current_inst.mult_vec == 3) {
        vx = GetIrVector();
    } else {
        vx = v[current_inst.mult_vec];
    }

    Vec3 tx{};
    if (current_inst.trans_vec == 0) {
        tx = trans_vec;
    } else if (current_inst.trans_vec == 1) {
        tx = bg_color;
    } else if (current_inst.trans_vec == 2) {
        tx = far_color;
        // TODO: fix bugged MVMVA for this case;
    }
    MultMatrixByVector(mx, vx, tx);
}

void GTE::RTPS(int vector) {
    MultMatrixByVector(rotation, v[vector], trans_vec);
    sz[3] = mac[3] >> ((1 - current_inst.sf) * 12);

    //perform the UNR division algo
    uint32_t n = 0;
    if (h < sz[3] * 2) {
        uint32_t z = GetLeadingZeroes(sz[3]);
        n = h << z;
        uint32_t d = sz[3] << z;
        uint32_t u = UNR_table[(d - 0x7FC0) >> 7] + 0x101;
        d = ((0x2000080 - (d * u)) >> 8);
        d = ((0x0000080 + (d * u)) >> 8);
        n = std::min((unsigned int)0x1FFFF, (((n * d) + 0x8000) >> 16));
    } else {
        n = 0x1FFFF;
        flag.div_overflow = 1;
        flag.error_flag = 1;
    }

    int32_t x = (((int64_t)n * ir[1]) + offset_x) >> 16;
    SetArithmeticFlags<32>(x, Flag::Mac0OverflowNegative, Flag::Mac0OverflowPositive);
    int32_t y = (n * ir[2] + offset_y) >> 16;
    SetArithmeticFlags<32>(y, Flag::Mac0OverflowNegative, Flag::Mac0OverflowPositive);
    PushScreenXY(x, y);

    mac[0] = n * dqa + dqb;
    SetArithmeticFlags<32>(mac[0], Flag::Mac0OverflowNegative, Flag::Mac0OverflowPositive);
    ir[0] = clamp(mac[0] >> 12, 0x1000, 0, Flag::Ir0Saturated);
}

void GTE::RTPT() {
    RTPS(0);
    RTPS(1);
    RTPS(2);
}

void GTE::NCLIP() {
    mac[0] = sxy[0].x * sxy[1].y + sxy[1].x * sxy[2].y + sxy[2].x * sxy[0].y 
        - sxy[0].x * sxy[2].y - sxy[1].x * sxy[0].y - sxy[2].x * sxy[1].y;
    SetArithmeticFlags<32>(mac[0], Flag::Mac0OverflowNegative, Flag::Mac0OverflowPositive);
}

void GTE::NCCS(int vector) {
    MultMatrixByVector(light_source, v[vector]);
    MultMatrixByVector(light_color, GetIrVector(), bg_color);
    MultVectorByVector(GetRGBCVector(), GetIrVector());
}

void GTE::NCCT() {
    NCCS(0);
    NCCS(1);
    NCCS(2);
}

void GTE::MultMatrixByVector(const Matrix& m, const Vec3& v, const Vec3& tr) {
    SetMacAndIr<1>((tr.x << 12) + (m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z));
    SetMacAndIr<2>((tr.y << 12) + (m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z));
    SetMacAndIr<3>((tr.z << 12) + (m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z));
}

void GTE::MultVectorByVector(const Vec3& v1, const Vec3& v2, const Vec3& tr) {
    SetMacAndIr<1>(tr.x << 12 + v1.x * v2.x);
    SetMacAndIr<2>(tr.y << 12 + v1.y * v2.y);
    SetMacAndIr<3>(tr.z << 12 + v1.z * v2.z);
}

constexpr std::array<uint8_t, 0x101> GTE::GenerateUNRTable() {
    std::array<uint8_t, 0x101> UNR_table{{0}};
    for (unsigned i = 0; i < UNR_table.size(); i++) {
        UNR_table[i] = std::min((unsigned)0, (0x40000/(i + 0x100) + 1) / 2 - 0x101);
    }
    return UNR_table;
}

int32_t GTE::clamp(int32_t val, int32_t max, int32_t min, Flag flags) {
    if (val > max) {
        flag.reg |= (uint32_t)flags;
        return max;
    }
    if (val < min) {
        flag.reg |= (uint32_t)flags;
        return min;
    }
    return val;
}

void GTE::PushScreenXY(int32_t x, int32_t y) {
    sxy[0].x = sxy[1].x;
    sxy[0].y = sxy[1].y;
    sxy[1].x = sxy[2].x;
    sxy[1].y = sxy[2].y;
    sxy[2].x = clamp(x, 0x3FF, -0x400, Flag::Sx2Saturated);
    sxy[2].y = clamp(y, 0x3FF, -0x400, Flag::Sx2Saturated);
}

GTE::Vec3 GTE::GetIrVector() const {
    return glm::vec<3, uint16_t> {ir[1], ir[2], ir[3]};
}

GTE::Vec3 GTE::GetRGBCVector() const {
    return glm::vec<3, uint16_t> {rgbc.r << 4, rgbc.g << 4, rgbc.b << 4};
}