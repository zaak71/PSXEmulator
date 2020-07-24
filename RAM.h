#pragma once

#include <array>
#include "Constants.h"

class RAM {
public:
    RAM();

    template <typename Value>
    Value Read(uint32_t offset) const {
        return *(Value*)(memory.data() + offset);
    }

    template <typename Value>
    void Write(uint32_t offset, Value data) {
        *(Value*)(memory.data() + offset) = data;
    }
private:
    std::array<uint8_t, RAM_SIZE> memory;
};

class Scratchpad {
public:
    Scratchpad();

    template <typename Value>
    Value Read(uint32_t offset) const {
        return *(Value*)(scratchpad.data() + offset);
    }

    template <typename Value>
    void Write(uint32_t offset, Value data) {
        *(Value*)(scratchpad.data() + offset) = data;
    }
private:
    std::array<uint8_t, SCRATCHPAD_SIZE> scratchpad;
};

