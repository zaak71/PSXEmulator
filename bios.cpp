#include "bios.h"

#include <fstream>

Bios::Bios(PSX* system) : system(system) {
}

void Bios::LoadBios(const std::string& file_path) {
    std::ifstream bios_file(file_path, std::ios::binary | std::ios::in);
    if (bios_file.is_open()) {
        bios_data.resize(512 * 1024);
        bios_file.read((char*)bios_data.data(), 512 * 1024);
    }
    bios_file.close();
}

uint32_t Bios::Read32(int index) const {
    return *((int32_t*)(bios_data.data() + index));
}