#pragma once

#define BIOS_START_ADDRESS      0x1FC00000
#define BIOS_SIZE               512 * 1024

#define RAM_START_ADDRESS       0x00000000
#define RAM_SIZE                2 * 1024 * 1024

#define MEM_CONTROL_1_START     0x1F801000
#define MEM_CONTROL_1_SIZE      0x24

#define MEM_CONTROL_2_START     0x1F801060
#define MEM_CONTROL_2_SIZE      0x4

#define CACHE_CONTROL_START     0xFFFE0130 
#define CACHE_CONTROL_SIZE      0x4

#define SCRATCHPAD_START        0x1F800000
#define SCRATCHPAD_SIZE         0x400

#define CDROM_START             0x1F801800
#define CDROM_SIZE              0x4

#define GPU_START               0x1F801810
#define GPU_SIZE                0x8

#define MDEC_START              0x1F801820
#define MDEC_SIZE               0x8

#define SPU_START               0x1F801C00
#define SPU_SIZE                0x280