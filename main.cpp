#include <iostream>
#include <string>
#include <vector>

#include "PSX.h"
#include "Constants.h"

int main(int argc, char** argv) {
    PSX system;
    int cycles = 1;
    while (true) {
        system.Run();
        cycles++;
    }
    return 0;
}