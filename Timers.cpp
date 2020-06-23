#include "Timers.h"

#include <cstdio>
#include <cassert>

void Timers::Write16(uint32_t offset, uint16_t data) {
	uint16_t timer = offset / 0x10;
	offset = offset % 0x10;
	switch (offset) {
		case 0:
			curr_counter_val[timer] = data;
			break;
		case 4:
			curr_counter_val[timer] = 0;
			counter_mode[timer].reg = data;
			break;
		case 8:
			target_val[timer] = data;
			break;
		default:
			printf("Unhandled write to Timer at offset %02x, data %08x\n", offset, data);
			assert(false);
			break;
	}
}

uint16_t Timers::Read16(uint32_t offset) {
	uint16_t timer = offset / 0x10;
	offset = offset % 0x10;
	switch (offset) {
		case 0:
			return curr_counter_val[timer];
			break;
		case 4:
			return counter_mode[timer].reg;
			break;
		case 8:
			return target_val[timer];
			break;
		default:
			printf("Unhandled read from Timer at offset %02x\n", offset);
			assert(false);
			return 0;
			break;
	}
}