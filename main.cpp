#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
    std::string bios_path = "bios/SCPH1001.BIN";
    std::ifstream bios_file(bios_path, std::ios::binary | std::ios::in);
    if (bios_file.is_open()) {
        std::cout << "success" << std::endl;
    }
    return 0;
}