#include "IRQ.h"

#include <cstdio>

void IRQ::Write32(uint32_t offset, uint32_t data) {
	switch (offset) {
		case 0:
			i_stat = data;
			break;
		case 4:
			i_mask.reg = data;
			break;
		default:
			break;
	}
}

void IRQ::Write16(uint32_t offset, uint16_t data) {
	Write32(offset, (uint32_t)data);
}

uint32_t IRQ::Read32(uint32_t offset) const {
	switch (offset) {
	case 0:
		return i_stat;
		break;
	case 4:
		return i_mask.reg;
		break;
	default:
		printf("Unhandled memory access of IRQ, size 32 at offset %01x\n", offset);
		return 0;
		break;
	}
}