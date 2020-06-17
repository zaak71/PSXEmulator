#pragma once

#include <cstdint>

class cop0 {
public:
    uint32_t Read(int reg_num);
    void Write(int reg_num, uint32_t data);

    union Cause {
        uint32_t reg = 0;
        struct flags {
            uint32_t _0_2 : 2;
            uint32_t exception_code : 5;
            uint32_t _7 : 1;
            uint32_t interrupt_pending : 8;
            uint32_t _16_27 : 12;
            uint32_t cop_exception : 2;
            uint32_t _30 : 1;
            uint32_t branch_delay : 1;
        } cause_flags;
    } cause_register;

    union Status {
        uint32_t reg = 0;
        struct flags {
            uint32_t current_interrupt_enable : 1;      // 0=Disable, 1=Enable
            uint32_t current_kernel_user_mode: 1;       // 0=Kernel, 1=User
            uint32_t prev_interrupt_enable : 1;         // 0=Disable, 1=Enable
            uint32_t prev_kernel_user_mode : 1;         // 0=Kernel, 1=User
            uint32_t old_interrupt_enable : 1;          // 0=Disable, 1=Enable
            uint32_t old_kernel_user_mode : 1;          // 0=Kernel, 1=User
            uint32_t _6_7 : 2;
            uint32_t interrupt_mask : 8;
            uint32_t isolate_cache : 1;                 // 0=No, 1=Isolate
            uint32_t swapped_cache_mode : 1;
            uint32_t pz : 1;
            uint32_t cm : 1;
            uint32_t pe : 1;
            uint32_t ts : 1;
            uint32_t bez : 1;
            uint32_t _23_27 : 5;
            uint32_t cop0_enable : 1;                   // 0=Only Kernel Mode, 1=Kernel+User
            uint32_t cop1_enable : 1;
            uint32_t cop2_enable : 1;                   // 0=Disable, 1=Enable
            uint32_t cop3_enable : 1;
        } status_flags;
    } status_register;

    uint32_t epc;
};

