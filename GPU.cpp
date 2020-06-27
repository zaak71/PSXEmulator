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

void GPU::Write32(uint32_t offset, uint32_t data) {
	switch (offset) {
	case 0:
		printf("GP0 Command (CPU): %08x\n", data);
		break;
	case 4:
		printf("GP1 Command (CPU): %08x\n", data);
		break;
	default:
		printf("Invalid access to GPU at offset %02x\n", offset);
		assert(false);
		break;
	}
}