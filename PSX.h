#pragma once

#include <memory>

#include "bios.h"
#include "CPU.h"

class PSX {
public:
    PSX();
    void Run();

    uint32_t Read32(const int address);
private:
    std::unique_ptr<Bios> sys_bios;
    std::unique_ptr<CPU> sys_cpu;
};

