#pragma once

#include "dwarf.cpp"

namespace kr1v {
    void evil() {
        dwarf::readDebugInfo();
    }
}

// void SYS_PreMain() {
//     printf("first");
// }