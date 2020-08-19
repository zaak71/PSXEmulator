#pragma once

#include <cstdint>
#include <array>

class MDEC {
public:
    void Write32(uint32_t offset, uint32_t data);
    uint32_t Read32(uint32_t offset);
private:
    void HandleCommand(uint32_t command);

    uint32_t param_num = 0;
    uint32_t params_left = 0;
    std::array<uint8_t, 64> luminance_quant_table {};
    std::array<uint8_t, 64> color_quant_table {};
    std::array<int16_t, 64> idct_table {};

    enum class MDECCommands {
        None = 0,
        DecodeMacroblock = 1,
        SetQuantTable = 2,
        SetScaleTable = 3
    } curr_cmd;

    union MDECCommand {
        uint32_t word = 0;
        struct {
            uint32_t data : 29;
            uint32_t opcode : 3;
        };
    };

    union MDECStatus {
        uint32_t reg = 0x80040000;
        struct {
            uint32_t param_words_minus_1 : 16;      // 0xFFFF=None
            uint32_t curr_block : 3;                // 0..3=Y1..Y4, 4=Cr, 5=Cb
            uint32_t : 4;
            uint32_t data_output_b15 : 1;           // 0=Clear, 1=Set
            uint32_t data_output_signed : 1;        // 0=Unsigned, 1=Signed
            uint32_t data_output_depth : 2;         // 4, 8, 24, 15 bits
            uint32_t data_out_request : 1;          // Set when DMA1 enabled
            uint32_t data_in_request : 1;           // Set when DMA0 enabled
            uint32_t cmd_busy : 1;                  // 0=Ready, 1=Busy
            uint32_t data_in_fifo_full : 1;         // 0=No, 1=Full
            uint32_t data_out_fifo_empty : 1;       // 0=No, 1=Empty
        };
    } status;

    union MDECControl {
        uint32_t reg = 0;
        struct {
            uint32_t : 29;
            uint32_t enable_data_out_request : 1;   // 0=Disable, 1=Enable
            uint32_t enable_data_in_request : 1;    // 0=Disable, 1=Enable
            uint32_t reset_mdec : 1;                // 0=No change, 1=Abort
        };
    } control;
};

