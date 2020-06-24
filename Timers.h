#pragma once

#include <cstdint>

class Timers {
public:
    void Write16(uint32_t offset, uint16_t data);
    uint16_t Read16(uint32_t offset) const;
    uint32_t Read32(uint32_t offset) const;
private:
    uint16_t curr_counter_val[3];
    union {
        uint16_t reg = 0;
        struct {
            uint16_t sync_enable : 1;   // 0=Free Run, 1=Sync via bit1-2
            uint16_t sync_mode : 2;     // Counter 0
                                        // 0=pause during hblank, 1=reset to 0 during hblank
                                        // 1=reset to 0 during hblank and pause outside
                                        // 2=pause until hblank occurs once, then switch to free run
                                        // Counter 1: same as 0, but vblank instead of hblank
                                        // Counter 2: 
                                        // 0/3=stop counter at curr val, 1/2=Free Run
            uint16_t reset : 1;         // 0=After Counter = FFFF, 1=After Counter = Target Value
            uint16_t irq_target : 1;    // 0=Disable, 1=Enable
            uint16_t irq_ffff : 1;      // 0=Disable, 1=Enable
            uint16_t repeat_mode : 1;   // 0=one-shot, 1=Repeatedly
            uint16_t toggle_mode : 1;   // 0=Short Bit 10=0 Pulse, 1=Toggle Bit 10
            uint16_t clk_src : 2;       // Counter 0: 0/2=Sys Clock, 1/3=Dotclock
                                        // Counter 1: 0/2=Sys Clock, 1/3=Hblank
                                        // Counter 2: 0/1=Sys Clock, 2/3=Sys Clock/8
            uint16_t irq : 1;           // 0=Yes, 1=No (Set After Writing)
            uint16_t reached_tgt : 1;   // 0=No, 1=Yes (Reset After Reading)
            uint16_t reached_ffff : 1;  // 0=No, 1=Yes (Reset After Reading)
            uint16_t : 3;
        } flags;
    } counter_mode[3];
    uint16_t target_val[3];
};

