#include "RAM.h"
#include <fstream>

RAM::RAM() {
    memory.fill(0);
}

void RAM::DumpRAM() {
    std::ofstream ram_file;
    ram_file.open("ram_file.bin", std::ios::binary | std::ios::out);
    ram_file.write((const char*)memory.data(), memory.size() * sizeof(uint8_t));
    ram_file.close();
}

Scratchpad::Scratchpad() {
    scratchpad.fill(0);
}