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