#include "bios.h"

#include <fstream>

Bios::Bios(PSX* system) : system(system) {
}

void Bios::LoadBios(const std::string& file_path) {
    std::ifstream bios_file(file_path, std::ios::binary | std::ios::in);
    if (bios_file.is_open()) {
        bios_file.read((char*)bios_data.data(), bios_data.size());
    }
    bios_file.close();
}

uint32_t Bios::Read32(int index) const {
    return *((int32_t*)(bios_data.data() + index));
}