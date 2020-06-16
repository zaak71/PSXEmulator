#pragma once

#include <string>
#include <vector>

class PSX;

class Bios {
public:
    Bios(PSX* system);
    void LoadBios(const std::string& file_path);

    uint32_t Read32(int index) const;
private:
    std::vector<uint8_t> bios_data;
    PSX* system = nullptr;
};

