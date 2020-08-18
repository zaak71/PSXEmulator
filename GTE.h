#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <array>

class GTE {
public:
    GTE();
    uint32_t Read(uint32_t reg_num);
    void Write(uint32_t reg_num, uint32_t data);
    void ExecuteGTECommand(uint32_t inst);
private:
    uint32_t GetConversionOutput();
    constexpr std::array<uint8_t, 0x101> GenerateUNRTable();
    const std::array<uint8_t, 0x101> UNR_table;

    union GTEInstruction {
        uint32_t inst = 0;
        struct {
            uint32_t cmd_num : 6;       // 00..3F
            uint32_t : 4;
            uint32_t lm : 1;            // 0=-0x8000...0x7FFF, 1=0...0x7FFF
            uint32_t : 2;
            uint32_t trans_vec : 2;     // 0=TR, 1=BK, 2=FC/Bugged, 3=None
            uint32_t mult_vec : 2;      // 0->2=V0->V2, 3=IR/Long
            uint32_t mult_mat : 2;      // 0=Rotation, 1=Light, 2=Color, 3=Reserved
            uint32_t sf : 1;            // 0=No fraction, 1=12 bit fraction
            uint32_t fake_cmd_num : 5;  // 00..1F
            uint32_t cop2_code : 7;     // Must be 0100101b for GTE
        };
    } current_inst;
    using Matrix = glm::mat<3, 3, int16_t>;
    using Vec3 = glm::vec<3, int16_t>;

    void RTPS(int vector);
    void MVMVA();
    void NCLIP();
    void NCCS(int vector);
    void RTPT();
    void NCCT();

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

    enum class Flag : uint32_t{
        Ir0Saturated = 1 << 12,
        Sy2Saturated = 1 << 13,
        Sx2Saturated = 1 << 14,
        Mac0OverflowPositive = 1 << 15,
        Mac0OverflowNegative = 1 << 16,
        DivisionOverflow = 1 << 17,
        Sz3OtzSaturated = 1 << 18,
        ColorFifoBSaturated = 1 << 19,
        ColorFifoGSaturated = 1 << 20,
        ColorFifoRSaturated = 1 << 21,
        Ir3Saturated = 1 << 22,
        Ir2Saturated = 1 << 23,
        Ir1Saturated = 1 << 24,
        Mac3OverflowNegative = 1 << 25,
        Mac2OverflowNegative = 1 << 26,
        Mac1OverflowNegative = 1 << 27,
        Mac3OverflowPositive = 1 << 28,
        Mac2OverflowPositive = 1 << 29,
        Mac1OverflowPositive = 1 << 30
    };

    // Utility Functions
    void MultMatrixByVector(const Matrix& m, const Vec3& v, const Vec3& tr = Vec3(0));
    void MultVectorByVector(const Vec3& v1, const Vec3& v2, const Vec3& tr = Vec3(0));
    int32_t clamp(int32_t val, int32_t max, int32_t min, Flag flags);
    void PushScreenXY(int32_t x, int32_t y);
    void PushColor(uint32_t r, uint32_t g, uint32_t b);
    Vec3 GetIrVector() const;
    Vec3 GetRGBCVector() const;

    template <int bits>
    void SetArithmeticFlags(int64_t val, Flag underflow_flag, Flag ov_flag) {
        if (val >= (1LL << (bits - 1))) {
            flag.reg |= (uint32_t)ov_flag;
        }
        if (val < -(1LL << (bits - 1))) {
            flag.reg |= (uint32_t)underflow_flag;
        }
    }

    template <int index>
    void SetMac(int64_t val) {
        if (index == 1) {
            SetArithmeticFlags<44>(val, Flag::Mac1OverflowNegative, Flag::Mac1OverflowPositive);
        } else if (index == 2) {
            SetArithmeticFlags<44>(val, Flag::Mac2OverflowNegative, Flag::Mac2OverflowPositive);
        } else if (index == 3) {
            SetArithmeticFlags<44>(val, Flag::Mac3OverflowNegative, Flag::Mac3OverflowPositive);
        }
        if (current_inst.sf) {
            val >>= 12;
        }
        mac[index] = (int32_t)val;
    }

    template <int index>
    void SetIr(int32_t val) {
        Flag flag;
        if constexpr (index == 1) {
            flag = Flag::Ir1Saturated;
        } else if constexpr (index == 2) {
            flag = Flag::Ir2Saturated;
        } else if constexpr (index == 3) {
            flag = Flag::Ir3Saturated;
        }
        ir[index] = clamp(val, 0x7FFF, current_inst.lm ? 0: -0x8000, flag);
    }

    template <int index>
    void SetMacAndIr(int32_t val) {
        SetMac<index>(val);
        SetIr<index>(val);
    }
    
    // Data Registers
    Vec3 v[3];                                  // r0-5
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
    Matrix rotation{};                          // r32-36
    Vec3 trans_vec;                             // r37-39
    Matrix light_source{};                      // r40-44
    Vec3 bg_color;                              // r45-47
    Matrix light_color{};                       // r48-52
    Vec3 far_color;                             // r53-55
    int32_t offset_x = 0;                       // r56
    int32_t offset_y = 0;                       // r57
    uint16_t h = 0;                             // r58
    int16_t dqa = 0;                            // r59
    int32_t dqb = 0;                            // r60
    int16_t zsf3 = 0;                           // r61
    int16_t zsf4 = 0;                           // r62
    GTEFlag flag;                               // r63
};

