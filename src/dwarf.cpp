#pragma once

#import <string>

#include <fat.h>
#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>

namespace kr1v::dwarf {
    static std::string dwarf_path = "sd:/dwarf/main.dwarf";

    std::vector<char> static readDebugInfo() {
        if (!fatInitDefault()) {
            printf("Failed to initialize SD card");
        }

        std::ifstream file(dwarf_path, std::ios::binary);

        if (!file) {
            printf("Failed to open main.dwarf");
        }

        return {
            std::istreambuf_iterator(file),
            std::istreambuf_iterator<char>()
        };
    }
}
