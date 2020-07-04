#include "IRQ.h"

#include <cstdio>
#include <cassert>

IRQ::IRQ(CPU* cpu) {
	this->cpu = cpu;
}

void IRQ::TriggerIRQ(int irq) {
	assert(irq >= 0 && irq <= 10);
	i_stat.reg |= (1 << irq);
	cpu->COP0.cause.interrupt_pending = (i_stat.reg & i_mask.reg) ? 0b100 : 0;
}

void IRQ::Write32(uint32_t offset, uint32_t data) {
	switch (offset) {
		case 0:
			i_stat.reg &= data;
			break;
		case 4:
			i_mask.reg = data;
			break;
		default:
			return;
			break;
	}
	cpu->COP0.cause.interrupt_pending = (i_stat.reg & i_mask.reg) ? 0b100 : 0;
}

void IRQ::Write16(uint32_t offset, uint16_t data) {
	Write32(offset, (uint32_t)data);
}

uint32_t IRQ::Read32(uint32_t offset) const {
	switch (offset) {
	case 0:
		return i_stat.reg;
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

uint16_t IRQ::Read16(uint32_t offset) const {
	switch (offset) {
	case 0:
		return i_stat.reg & 0x0000FFFF;
	case 4:
		return i_mask.reg & 0x0000FFFF;
		break;
	default:
		printf("Unhandled memory access of IRQ, size 16 at offset %01x\n", offset);
		return 0;
		break;
	}
}