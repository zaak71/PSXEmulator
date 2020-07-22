#include "Timers.h"

#include <cstdio>
#include <cassert>

void Timers::Cycle(int cycles) {
	for (int i = 0; i < 3; i++) {
		uint32_t& val = curr_counter_val[i];
		uint16_t& target = target_val[i];
		TimerMode& mode = counter_mode[i];
		val += cycles;

		if (val > target) {
			mode.reached_tgt = 1;
			if (mode.irq_target) {

			}
			if (mode.reset) {
				val = 0;
			}
		}
		if (val > 0xFFFF) {
			mode.reached_ffff = 1;
			if (mode.irq_target) {

			}
			if (!mode.reset) {
				val = 0;
			}
		}
	}
}

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
			counter_mode[timer].irq = 1;
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

void Timers::Write32(uint32_t offset, uint32_t data) {
	Write16(offset, data & 0xFFFFu);
}

uint16_t Timers::Read16(uint32_t offset) {
	uint16_t timer = offset / 0x10;
	offset = offset % 0x10;
	uint16_t data = 0;
	switch (offset) {
		case 0:
			return curr_counter_val[timer] & 0xFFFFu;
			break;
		case 4:
			data = counter_mode[timer].reg;
			counter_mode[timer].reached_ffff = 0;
			counter_mode[timer].reached_tgt = 0;
			return data;
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

uint32_t Timers::Read32(uint32_t offset) {
	return Read16(offset);
}