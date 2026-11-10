#pragma once

#include "dwarf.h"
#include "patch.h"

namespace kr1v {
    namespace patch {
        void transform(std::vector<int>* in);
    }

    void evil() {
        if (!dwarf::readDebugInfo()) {
            printf("DWARF failed to read, aborting.");
            return;
        }
        patch::doIt(dwarf::getFunctionByName("test")[0]);
    }
}

// void SYS_PreMain() {
//     printf("first");
// }