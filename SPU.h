#pragma once

#include <cstdint>

class SPU {
public:
    void Write16(uint32_t address, uint16_t data);
    uint16_t Read16(uint32_t address) const;
private:
    uint16_t main_volume_l = 0;
    uint16_t main_volume_r = 0;
    uint16_t vLOUT = 0;
    uint16_t vROUT = 0;

    union SPUControl {  // 0x1F801DAA
        uint16_t reg = 0;
        struct {
            uint16_t CD_audio_enable : 1;
            uint16_t external_audio_enable : 1;
            uint16_t CD_audio_reverb : 1;
            uint16_t external_audio_reverb : 1;
            uint16_t sram_transfer_mode : 2;    // 0=Stop, 1=ManualWrite, 2=DMAWrite, 3=DMARead
            uint16_t irq9_enable : 1;
            uint16_t reverb_master_enable : 1;
            uint16_t noise_freq_step : 2;       // 0...3=4...7
            uint16_t noise_freq_shift : 4;      // 0...F= Low to High Frequency
            uint16_t mute_spu : 1;             // 0=Mute, 1=Unmute
            uint16_t spu_enable : 1;
        } flags;
    } SPUCNT;

    union SPUStatus {   // 0x1F801DAE
        uint16_t reg = 0;
        struct {
            uint16_t current_mode : 6;
            uint16_t irq9_flag : 1;
            uint16_t data_transfer_dma_rw_req : 1;
            uint16_t data_transfer_dma_w_req : 1;   // 0=No, 1=Yes
            uint16_t data_transfer_dma_r_req : 1;   // 0=No, 1=Yes
            uint16_t data_transfer_busy_flag : 1;   // 0=Ready, 1=Busy
            uint16_t write_to_half_capture_buf: 1;  // 0=1st half, 1=2nd half
            uint16_t : 4;
        } flags;
    } SPUSTAT;

    union KeyOff {
        uint32_t reg = 0;
        uint16_t halves[2];
        struct {
            uint32_t voice_off : 24;    // 0=No Change, 1=Start Release
            uint32_t : 8;
        } flags;
    } key_off;

    union KeyOn {
        uint32_t reg = 0;
        uint16_t halves[2];
        struct {
            uint32_t voice_on : 24;     // 0=No Change, 1=Start Attack/Decay/Sustain
            uint32_t : 8;
        } flags;
    } key_on;

    union PitchModulationFlags {
        uint32_t reg = 0;
        uint16_t halves[2];
        struct {
            uint32_t : 1;
            uint32_t voice_flags : 23;     // 0=Normal, 1=Modulate by Voice 0..22
            uint32_t : 8;
        } flags;
    } PMON;

    union NoiseModeEnable {
        uint32_t reg = 0;
        uint16_t halves[2];
        struct {
            uint32_t voice_noise : 24;     // 0=ADPCM, 1=Noise
            uint32_t : 8;
        } flags;
    } NON;

    union ReverbMode {
        uint32_t reg = 0;
        uint16_t halves[2];
        struct {
            uint32_t voice_dest : 24;     // 0=Mixer, 1=Mixer and Reverb
            uint32_t : 8;
        } flags;
    } EON;

    struct Volume {
        uint16_t left;      // -8000h to 7FFFh
        uint16_t right;
    } CD_input_volume, external_input_volume;

    union SRAMDataTransferControl {
        uint16_t reg;
        struct {
            uint16_t : 1;
            uint16_t sram_data_transfer_type : 3;
            uint16_t : 12;
        } flags;
    } sram_data_transfer_control;

    uint16_t sram_data_transfer_address = 0;
    uint16_t sram_data_transfer_fifo = 0;

    struct Voice {
        uint16_t volume_left;
        uint16_t volume_right;
        uint16_t adpcm_sample_rate;
        uint16_t adpcm_start_addr;
        uint16_t adsr_lower;
        uint16_t adsr_upper;
        uint16_t adsr_curr_vol;
        uint16_t adpcm_repeat_addr;
    };
    Voice voices[24];
    void HandleVoiceWrite(uint16_t offset, uint16_t voice, uint16_t data);
};

