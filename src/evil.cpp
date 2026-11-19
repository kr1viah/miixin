#pragma once

#include "dwarf.h"
#include "patch.h"

namespace kr1v {
    void evil() {
        if (!dwarf::readDebugInfo()) {
            printf("DWARF failed to read, aborting.");
            return;
        }
        patch::doIt(dwarf::getFunctionByName("test2")[0]);
    }
}

// void SYS_PreMain() {
//     printf("first");
// }