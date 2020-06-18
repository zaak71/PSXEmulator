#include "SPU.h"
#include "Constants.h"

#include <cstdio>

void SPU::Write16(uint32_t address, uint16_t data) {
	switch (address & 0x00000FFF) {
		case 0xD80:
			main_volume_l = data;
			break;
		case 0xD82:
			main_volume_r = data;
			break;
		case 0xD84:
			vLOUT = data;
			break;
		case 0xD86:
			vROUT = data;
			break;
		default:
			printf("Unhandled write to SPU at address %08x\n", address);
			break;
	}
}