#pragma once

#include <string>
#include <array>

class PSX;

class Bios {
public:
    Bios(PSX* system);
    void LoadBios(const std::string& file_path);

    uint32_t Read32(int index) const;
private:
    std::array<uint8_t, 512 * 1024> bios_data;
    PSX* system = nullptr;
};

