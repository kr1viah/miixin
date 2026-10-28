#pragma once

#include "dwarf.cpp"

namespace kr1v {
    void evil() {
        auto debug = dwarf::readDebugInfo();
        printf("%c%c%c%c\n", debug[0], debug[1], debug[2], debug[3]);
    }
}