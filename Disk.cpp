#include "Disk.h"

void Disk::LoadGame(const std::string& path) {
    game_file = std::ifstream(path, std::ios::binary | std::ios::in);
    game_file.seekg(0, game_file.end);
    int size = game_file.tellg();
    game_file.seekg(0, game_file.beg);

    data.resize(size);
    if (game_file.is_open()) {
        game_file.read((char*)data.data(), size);
    }
    game_file.close();
}

std::vector<uint8_t> Disk::read(uint32_t pos) {
    std::vector<uint8_t> read_data(2352);
    game_file.seekg(pos);
    game_file.read((char*)read_data.data(), 2352);
    int offset = pos * 2352;
    read_data.assign((data.begin() + offset), data.begin() + offset + 2352);
    return read_data;
}