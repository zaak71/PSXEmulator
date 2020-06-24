#include "GPU.h"
#include <cassert>
#include <cstdio>

uint32_t GPU::Read32(uint32_t offset) const {
	switch (offset) {
		case 0:
			return 0x00000000;
			break;
		case 4:
			return GPUSTAT.reg;
			break;
		default:
			printf("Invalid access to GPU at offset %02x\n", offset);
			assert(false);
			return 0;
			break;
	}
}