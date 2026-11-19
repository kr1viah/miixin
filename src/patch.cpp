#include <stdexcept>
#include <vector>

#include "dwarf.h"
namespace kr1v::patch {
    namespace data {
        struct instruction {
#define INSTRUCTION_UNKNOWN 0
#define INSTRUCTION_bl 1
            int type;
            union {
                unsigned int raw;
                unsigned long whereDoesItJumpTo;
            };
        };

        bool isBl(unsigned int instruction) {
            return instruction >> 26 == 18 && instruction & 0b1;
        }

        unsigned long whereDoesItGoTo(unsigned long insAddr, unsigned int ins) {
            int LI = ((int)(ins << 6) >> 8) << 2;
            return insAddr + LI;
        }

        void rawToInstructions(unsigned long startAddr, std::vector<unsigned int>* raw, std::vector<instruction>* instructions) {
            for (int i = 0; i < raw->size(); i++) {
                unsigned int ins = (*raw)[i];
                if (isBl(ins)) {
                    unsigned long insAddr = startAddr + i*4;
                    unsigned long jumpAddr = whereDoesItGoTo(insAddr, ins);
                    instruction proper = {.type = INSTRUCTION_bl, .whereDoesItJumpTo = jumpAddr};
                    instructions->push_back(proper);
                } else {
                    instruction proper = {.type = INSTRUCTION_UNKNOWN, .raw = ins};
                    instructions->push_back(proper);
                }
            }
        }

        void instructionsToRaw(unsigned long startAddr, std::vector<unsigned int>* output, std::vector<instruction>* instructions) {
            output->clear();
            for (int i = 0; i < instructions->size(); i++) {
                instruction& ins = instructions->at(i);
                if (ins.type == INSTRUCTION_bl) {
                    unsigned int LI = ((unsigned int)((long)ins.whereDoesItJumpTo - (startAddr + i * 4)) >> 2) & 0b00000000111111111111111111111111;
                    unsigned int bl = 18 << 26 | (LI << 2) | 0b01;
                    output->push_back(bl);
                } else {
                    output->push_back(ins.raw);
                }
            }
        }
    }

    void transform(std::vector<data::instruction>* in) {
        for (data::instruction& i : *in) {
            if (i.type == INSTRUCTION_bl) {
                dwarf::function f = dwarf::getFunctionByName("redirect")[0];
                i.whereDoesItJumpTo = f.startAddr;
            }
        }
    }

    bool doIt(dwarf::function function) {
        std::vector<unsigned int> raw;
        for (int i = 0; i < function.size; i += 4) {
            raw.push_back(*function.startPointer(i));
        }

        std::vector<data::instruction> instructions;
        data::rawToInstructions(function.startAddr, &raw, &instructions);
        transform(&instructions);

        if (instructions.size()*4 <= function.size) {
            data::instructionsToRaw(function.startAddr, &raw, &instructions);
            // in-place
            for (int i = 0; i < raw.size(); i++) {
                *function.startPointer(i*4) = raw[i];
            }
        } else {
            throw std::runtime_error("not implemented");
            printf("old size = %ld, new size = %d\n", function.size, raw.size());
            // move to new place
            auto* newPlace = static_cast<unsigned int *>(
                // todo: this doesnt work... i dont know how to fix it yet... something with reducing mem1 heap and putting the function there
                malloc(function.size)
            );
            auto newAddr = reinterpret_cast<unsigned long>(newPlace);
            data::instructionsToRaw(newAddr, &raw, &instructions);
            // printf("new addr %lx, start addr %lx, diff %lx\n", newAddr, function.startAddr, function.startAddr - newAddr);
            // *function.startPointer() = INS_bl & newAddr - function.startAddr; // replace function with branch to new function
        }

        return true;
    }
}
