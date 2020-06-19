#include <iostream>
#include <string>
#include <vector>

#include "PSX.h"
#include "Constants.h"

int main(int argc, char** argv) {
    PSX system;
    for (int i = 0; i < 10000000; i++) {
        system.Run();
    }
    return 0;
}