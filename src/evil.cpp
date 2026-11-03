#pragma once

#include "dwarf.h"

namespace kr1v {
    void evil() {
        dwarf::readDebugInfo();
    }
}

// void SYS_PreMain() {
//     printf("first");
// }