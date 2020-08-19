#include "MDEC.h"
#include <cassert>
#include <cstdio>

void MDEC::Write32(uint32_t offset, uint32_t data) {
    switch (offset) {
        case 0:
            if (params_left == 0) {
                HandleCommand(data);
                status.param_words_minus_1 = (params_left - 1) & 0xFFFF;
            } else {
                switch (curr_cmd) {
                    case MDEC::MDECCommands::None:
                        break;
                    case MDEC::MDECCommands::DecodeMacroblock:
                        break;
                    case MDEC::MDECCommands::SetQuantTable: {
                        uint8_t* table_ptr = NULL;
                        size_t base = 0;
                        if (param_num < 64 / 4) {
                            base = param_num * 4;
                            table_ptr = luminance_quant_table.data();
                        } else if (param_num < 128 / 4) {
                            base = (param_num - 64 / 4) * 4;
                            table_ptr = color_quant_table.data();
                        }

                        for (int i = 0; i < 4; i++) {
                            table_ptr[base + i] = data >> (i * 8);
                        }
                        break;
                    }
                    case MDEC::MDECCommands::SetScaleTable:
                        break;
                    default:
                        break;
                }
                param_num++;
                params_left--;
                status.param_words_minus_1 = (params_left - 1) & 0xFFFF;
            }
            break;
        case 4:
           control.reg = data;
           if (control.reset_mdec) {
                status.reg = 0x80040000;
                status.data_in_request = control.enable_data_in_request;
                status.data_out_request = control.enable_data_out_request;
           }
           break;
        default: 
            printf("Unhandled write to MDEC at offset %01x, data %08x\n", offset, data);
            assert(false);
    }
}

uint32_t MDEC::Read32(uint32_t offset) {
    switch (offset) {
        case 4:
            status.data_in_fifo_full = 0;
            status.cmd_busy = 0;
            return status.reg;
            break;
        default:
            printf("Unhandled read from MDEC at offset %01x\n", offset);
            assert(false);
            return 0;
            break;
    }
}

void MDEC::HandleCommand(uint32_t command) {
    MDECCommand cmd {command};
    switch (cmd.opcode) {
        case 0x02:
            curr_cmd = MDECCommands::SetQuantTable;
            if (command & 1) {
                params_left = 128 / 4;  // 128 bytes
            } else {
                params_left = 64 / 4;   // 64 bytes
            }
            status.cmd_busy = 1;
            break;
        default:
            printf("Unhandled MDEC opcode: %01x\n", cmd.opcode);
            assert(false);
            break;
    }
}