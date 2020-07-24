#include "cop0.h"

#include <cstdio>
#include <assert.h>

uint32_t cop0::Read(int reg_num) {
    switch (reg_num) {
        case 3:
            return bpc_reg;
            break;
        case 5:
            return bda_reg;
            break;
        case 6:
            return jump_dest;
            break;
        case 7:
            return dcic_register;
            break;
        case 8:
            return bad_vaddr;
        case 9:
            return bdam_reg;
            break;
        case 11:
            return bpcm_reg;
            break;
        case 12:
            return status.reg;
            break;
        case 13:
            return cause.reg;
            break;
        case 14:
            return epc;
            break;
        case 15:
            return processor_id;
            break;
        default:
            printf("Unhandled read from register %02x\n", reg_num);
            assert(false);
            return 0;
            break;
        }
}

void cop0::Write(int reg_num, uint32_t data) {
    Cause reg;
    reg.reg = data;
    switch (reg_num) {
        case 3:
            bpc_reg = data;
            break;
        case 5:
            bda_reg = data;
            break;
        case 6:
            jump_dest = data;
            break;
        case 7:
            dcic_register = data;
            break;
        case 9:
            bdam_reg = data;
            break;
        case 11:
            bpcm_reg = data;
            break;
        case 12:
            status.reg = data;
            break;
        case 13:
            // only the first two bits of the interrupt pending field can be written
            cause.interrupt_pending &= 0b11;
            cause.interrupt_pending |= (reg.interrupt_pending & 0b11);
            break;
        case 14:
            epc = data;
            break;
        default:
            printf("Unhandled write to COP0 register %02x\n", reg_num);
            assert(false);
            break;
    }
}
