#include <vector>
#include <ogc/video.h>

#include "dwarf.h"
#define INS_bl 0x4BFF0000
#define INS_ori_r9_r9 0x61290000
#define INS_lis_r9 0x3D200000

namespace kr1v::patch {
    void transform(std::vector<int>* in) {
        in->at(1) = INS_lis_r9 | 0;
        in->at(2) = INS_ori_r9_r9 | 69;
    }

    bool doIt(dwarf::function function) {
        std::vector<int> instructions;
        for (int i = 0; i < function.size; i += 4) {
            instructions.push_back(*function.startPointer(i));
        }

        transform(&instructions);

        if (instructions.size()*4 <= function.size) {
            // in-place
            for (int i = 0; i < instructions.size(); i++) {
                *function.startPointer(i*4) = instructions[i];
            }
        } else {
            printf("old size = %ld, new size = %d\n", function.size, instructions.size());
            // move to new place
            int* newPlace = static_cast<int *>(
                // todo: this doesnt work... i dont know how to fix it yet... something with reducing mem1 heap and putting the function there
                malloc(function.size)
            );
            auto addr = reinterpret_cast<unsigned long>(newPlace);
            printf("new addr %lx, start addr %lx, diff %lx\n", addr, function.startAddr, function.startAddr - addr);
            *function.startPointer() = INS_bl & addr - function.startAddr; // replace function with branch to new function
        }

        return true;
    }
}
