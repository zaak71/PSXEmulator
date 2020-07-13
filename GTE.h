#pragma once

#include <cstdint>
#include <glm/glm.hpp>

class GTE {
public:
    uint32_t Read(uint32_t reg_num);
    void Write(uint32_t reg_num, uint32_t data);
private:
    uint32_t GetConversionOutput();

    union GTEFlag {
        uint32_t reg = 0;
        struct {
            uint32_t : 12;
            uint32_t ir0_sat : 1;           // saturated to 0...0x1000
            uint32_t sy2_sat : 1;           // saturated to -0x400...0x3FF
            uint32_t sx2_sat : 1;           // saturated to -0x400...0x3FF
            uint32_t mac0_overflow_neg : 1; // larger than 31 bits
            uint32_t mac0_overflow_pos : 1; // larger than 31 bits
            uint32_t div_overflow : 1;      // RTPS/RTPT div saturated to 0x1FFFF
            uint32_t sz3_otz_saturated : 1; // SZ3/OTZ saturated from 0...0xFFFF
            uint32_t color_fifo_b_sat : 1;  // saturated from 0...0xFF
            uint32_t color_fifo_g_sat : 1;  // saturated from 0...0xFF
            uint32_t color_fifo_r_sat : 1;  // saturated from 0...0xFF
            uint32_t ir3_saturated : 1;     // saturated from 0(lm=1) or -0x8000(lm=0) to 0x7FFF
            uint32_t ir2_saturated : 1;     // saturated from 0(lm=1) or -0x8000(lm=0) to 0x7FFF
            uint32_t ir1_saturated : 1;     // saturated from 0(lm=1) or -0x8000(lm=0) to 0x7FFF
            uint32_t mac3_overflow_neg : 1; // larger than 43 bits
            uint32_t mac2_overflow_neg : 1; // larger than 43 bits
            uint32_t mac1_overflow_neg : 1; // larger than 43 bits
            uint32_t mac3_overflow_pos : 1; // larger than 43 bits
            uint32_t mac2_overflow_pos : 1; // larger than 43 bits
            uint32_t mac1_overflow_pos : 1; // larger than 43 bits
            uint32_t error_flag : 1;        // bits 30-23 and 18-13 ORed together
        };
        uint16_t read() {
            error_flag = (0x7F87E000 & reg) != 0;
            return reg & 0xFFFFF000;
        }
    };

    // Data Registers
    glm::vec<3, int16_t> v[3];                  // r0-5
    glm::vec<4, uint8_t> rgbc;                  // r6
    uint16_t average_z = 0;                     // r7
    int16_t ir[4] = {0, 0, 0, 0};               // r8-11
    glm::vec<2, int16_t> sxy[4];                // r12-15
    uint16_t sz[4] = {0, 0, 0, 0};              // r16-19
    glm::vec<4, uint8_t> rgb[3];                // r20-22
    uint32_t res1;                              // r23
    int32_t mac[4] = {0, 0, 0, 0};              // r24-27
    uint16_t irgb = 0;                          // r28-29
    int32_t lzcs = 0;                           // r30
    int32_t lzcr = 0;                           // r31

    // Control Registers
    glm::mat<3, 3, int16_t> rotation{};         // r32-36
    glm::vec<3, int32_t> trans_vec;             // r37-39
    glm::mat<3, 3, int16_t> light_source{};     // r40-44
    glm::vec<3, int32_t> bg_color;              // r45-47
    glm::mat<3, 3, int16_t> light_color{};      // r48-52
    glm::vec<3, int32_t> far_color;             // r53-55
    int32_t offset_x = 0;                       // r56
    int32_t offset_y = 0;                       // r57
    uint16_t h = 0;                             // r58
    int16_t dqa = 0;                            // r59
    int32_t dqb = 0;                            // r60
    int16_t zsf3 = 0;                           // r61
    int16_t zsf4 = 0;                           // r62
    GTEFlag flag;                               // r63
};

