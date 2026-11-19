#pragma once
#include <string>
#include <vector>

namespace kr1v::dwarf {
    struct function {
        std::string name;
        std::string mangled;
        std::string namespace_;
        unsigned long startAddr = 0;
        unsigned long size = 0; // size in bytes

        unsigned int* startPointer() {
            return reinterpret_cast<unsigned int*>(startAddr);
        }

        unsigned int* startPointer(int offset) {
            return reinterpret_cast<unsigned int*>(startAddr + offset);
        }
    };

    const function EMPTY_FUNC = {};

    function getFunctionByMangled(std::string mangled);
    function getFunctionByAddress(unsigned long address);
    // if size > 1 functions will have different arguments
    std::vector<function> getFunctionByName(std::string namespace_name);
    std::vector<function> getFunctionByName(std::string namespace_, std::string name);

    bool readDebugInfo();
}
