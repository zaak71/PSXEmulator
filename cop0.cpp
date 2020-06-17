#include "cop0.h"

#include <cstdio>
#include <assert.h>

uint32_t cop0::Read(int reg_num) {
    switch (reg_num) {
    case 12:
        return status_register.reg;
        break;
    case 13:
        return cause_register.reg;
        break;
    case 14:
        return epc;
        break;
    default:
        printf("Unhandled read from register %02x", reg_num);
        assert(false);
        return 0;
        break;
    }
}

void cop0::Write(int reg_num, uint32_t data) {
    switch (reg_num) {
    case 12:
        status_register.reg = data;
        break;
    case 13:
        cause_register.reg = data;
        break;
    case 14:
        epc = data;
        break;
    default:
        printf("Unhandled write to register %02x", reg_num);
        assert(false);
        break;
    }
}
