#pragma once

#include <string>
#include <vector>
#include <fstream>

class Disk {
public:
    void LoadGame(const std::string& path);
    std::vector<uint8_t> read(uint32_t pos);
private:
    std::ifstream game_file{};
    std::vector<uint8_t> data{};
};

