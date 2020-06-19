#include "IRQ.h"

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