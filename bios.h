#pragma once

#include <string>
#include <array>

class PSX;

class Bios {
public:
    Bios(PSX* system);
    void LoadBios(const std::string& file_path);


    template <typename Value>
    Value Read(uint32_t offset) const {
        return *(Value*)(bios_data.data() + offset);
    }
private:
    std::array<uint8_t, 512 * 1024> bios_data = {};
    PSX* system = nullptr;
};

