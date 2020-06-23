#include "DMAChannel.h"

bool DMAChannel::IsActive() const {
	if (control.flags.sync_mode == SyncMode::Manual) {
		return control.flags.enable && control.flags.start_trigger;
	} else {
		return control.flags.enable;
	}
}

uint32_t DMAChannel::GetTransferLength() const {
	switch (control.flags.sync_mode) {
		case SyncMode::Manual:
			return dma_block_control & 0xFFFF;
			break;
		case SyncMode::Block:
			return (dma_block_control & 0xFFFF) * ((dma_block_control >> 16) & 0xFFFF);
			break;
		case SyncMode::LinkedList:	// should never reach here
		default:
			printf("Unhandled Transfer Length!\n");
			return 0;
			break;
	}
}

void DMAChannel::FinishTransfer() {
	control.flags.enable = 0;
	control.flags.start_trigger = 0;
}

const char* DMAChannel::SyncModeToString(SyncMode sm) {
	switch (sm) {
		case SyncMode::Manual:
			return "Manual";
			break;
		case SyncMode::Block:
			return "Block";
			break;
		case SyncMode::LinkedList:
			return "Linked List";
			break;
		default:	//should never reach here
			return "None";
			break;
	}
}

const char* DMAChannel::TransferDirToString(TransferDirection td) {
	switch (td) {
		case TransferDirection::FromRAM:
			return "From RAM";
			break;
		case TransferDirection::ToRAM:
			return "To RAM";
			break;
		default:	// should never reach here
			return "None";
			break;
	}
}