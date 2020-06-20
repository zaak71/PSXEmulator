#include "SPU.h"
#include "Constants.h"

#include <cstdio>
#include <cassert>

SPU::SPU() {
	for (int i = 0; i < 32; i++) {
		reverb_regs[i] = 0;
	}
}

void SPU::Write16(uint32_t address, uint16_t data) {
	uint32_t lsbs = address & 0x00000FFF;	// get least sig 3 bits
	if (lsbs >= 0xC00 && lsbs + 2 <= 0xD80) {
		HandleVoiceWrite(address & 0x0F, lsbs / 0x10, data);
		return;
	} else if (lsbs >= 0xDC0 && lsbs + 2 <= 0xE00) {
		uint32_t reg = (lsbs - 0xDC0) / 0x2;
		reverb_regs[reg] = data;
		return;
	}
	switch (lsbs) {
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
		case 0xD88:
			key_on.halves[0] = data;
			break;
		case 0xD8A:
			key_on.halves[1] = data;
			break;
		case 0xD8C:
			key_off.halves[0] = data;
			break;
		case 0xD8E:
			key_off.halves[1] = data;
			break;
		case 0xD90:
			PMON.halves[0] = data;
			break;
		case 0xD92:
			PMON.halves[1] = data;
			break;
		case 0xD94:
			NON.halves[0] = data;
			break;
		case 0xD96:
			NON.halves[1] = data;
			break;
		case 0xD98:
			EON.halves[0] = data;
			break;
		case 0xD9A:
			EON.halves[1] = data;
			break;
		case 0xDA2:
			mBASE = data;
			break;
		case 0xDA6:
			sram_data_transfer_address = data;
			break;
		case 0xDA8:
			sram_data_transfer_fifo = data;
			break;
		case 0xDAA:
			SPUCNT.reg = data;
			break;
		case 0xDAC:
			sram_data_transfer_control.reg = data;
			break;
		case 0xDB0:
			CD_input_volume.left = data;
			break;
		case 0xDB2:
			CD_input_volume.right = data;
			break;
		case 0xDB4:
			external_input_volume.left = data;
			break;
		case 0xDB6:
			external_input_volume.right = data;
			break;
		default:
			printf("Unhandled write to SPU at address %08x\n", address);
			assert(false);
			break;
	}
}

uint16_t SPU::Read16(uint32_t address) const {
	uint32_t lsbs = address & 0x00000FFF;	// get 3 least sig bits
	if (lsbs >= 0xC00 && lsbs + 2 <= 0xD80) {
		return ReadVoice(address & 0x0F, lsbs / 0x10);
	}
	switch (lsbs) {
		case 0xD88:
			return key_on.halves[0];
			break;
		case 0xD8A:
			return key_on.halves[1];
			break;
		case 0xD8C:
			return key_off.halves[0];
			break;
		case 0xD8E:
			return key_off.halves[1];
			break;
		case 0xDAA:
			return SPUCNT.reg;
			break;
		case 0xDAC:
			return sram_data_transfer_control.reg;
			break;
		case 0xDAE:
			return SPUSTAT.reg;
			break;
		default:
			printf("Unhandled read from SPU at address %08x\n", address);
			assert(false);
			return 0;
			break;
	}
}

void SPU::HandleVoiceWrite(uint16_t offset, uint16_t voice, uint16_t data) {
	switch (offset) {
		case 0:
			voices[voice].volume_left = data;
			break;
		case 2:
			voices[voice].volume_left = data;
			break;
		case 4:
			voices[voice].adpcm_sample_rate = data;
			break;
		case 6:
			voices[voice].adpcm_start_addr = data;
			break;
		case 8:
			voices[voice].adsr_lower = data;
			break;
		case 10:
			voices[voice].adsr_upper = data;
			break;
		case 12:
			voices[voice].adsr_curr_vol = data;
			break;
		case 14:
			voices[voice].adpcm_repeat_addr = data;
			break;
		default:
			printf("Unhandled write to SPU at offset %08x\n", offset);
			assert(false);
			break;
	}
}

uint16_t SPU::ReadVoice(uint16_t offset, uint16_t voice) const {
	switch (offset) {
	case 0:
		return voices[voice].volume_left;
		break;
	case 2:
		return voices[voice].volume_left;
		break;
	case 4:
		return voices[voice].adpcm_sample_rate;
		break;
	case 6:
		return voices[voice].adpcm_start_addr;
		break;
	case 8:
		return voices[voice].adsr_lower;
		break;
	case 10:
		return voices[voice].adsr_upper;
		break;
	case 12:
		return voices[voice].adsr_curr_vol;
		break;
	case 14:
		return voices[voice].adpcm_repeat_addr;
		break;
	default:
		printf("Unhandled read from SPU at offset %08x\n", offset);
		assert(false);
		return 0;
		break;
	}
}
