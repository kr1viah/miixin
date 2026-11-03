#pragma once

#include <string>

#include <fat.h>
#include <fstream>
#include <vector>
#include <map>
#include "../lib/dwarf.h"
#include "../lib/libdwarf.h"
#include "dwarf.h"

namespace kr1v {
    void evil();
}

namespace kr1v::dwarf {
    bool isScopeTag(Dwarf_Half tag) {
        return tag == DW_TAG_namespace
            || tag == DW_TAG_class_type
            || tag == DW_TAG_structure_type
            || tag == DW_TAG_union_type;
    }

    const char* dwarf_path = "sd:/dwarf/main.dwarf";

    struct function_info {
        std::string name;
        std::string mangled;
        std::string namespace_;
    };

    struct pending_function {
        function implementation;
        Dwarf_Off referenceOffset = 0;
        bool hasReference = false;
    };

    std::map<Dwarf_Off, function_info> subprogramInfoByOffset;
    std::vector<pending_function> pendingFunctions;

    std::vector<function> functions;
    std::map<unsigned long, function> addr2funcMap;

    std::string getMangled(Dwarf_Die die, Dwarf_Error* error) {
        Dwarf_Attribute attr = nullptr;

        int result = dwarf_attr(die, DW_AT_linkage_name, &attr, error);
        if (result == DW_DLV_NO_ENTRY) {
            result = dwarf_attr(die, DW_AT_MIPS_linkage_name, &attr, error);
        }

        if (result != DW_DLV_OK) {
            return "";
        }

        char* name = nullptr;
        result = dwarf_formstring(attr, &name, error);

        dwarf_dealloc_attribute(attr);

        return result == DW_DLV_OK ? name : "";
    }

    std::string getNamespace(const std::vector<std::string>& scopes) {
        std::string result;

        for (const std::string& scope : scopes) {
            if (!result.empty()) {
                result += "::";
            }

            result += scope;
        }

        return result;
    }

    bool getReferenceOffset(Dwarf_Die die, Dwarf_Half attribute, Dwarf_Off& offset, Dwarf_Error* error) {
        Dwarf_Attribute attr = nullptr;

        int result = dwarf_attr(die, attribute, &attr, error);

        if (result != DW_DLV_OK) {
            return false;
        }

        Dwarf_Bool isInfo = false;

        result = dwarf_global_formref_b(attr,  &offset, &isInfo, error);

        dwarf_dealloc_attribute(attr);

        return result == DW_DLV_OK && isInfo;
    }

    bool hasAttr(Dwarf_Die die, Dwarf_Half attrCode, Dwarf_Error* error) {
        Dwarf_Attribute attr = nullptr;
        int result = dwarf_attr(die, attrCode, &attr, error);

        if (result == DW_DLV_OK) {
            dwarf_dealloc_attribute(attr);
            return true;
        }

        return false;
    }

    void inspectFunction(Dwarf_Die die, std::vector<std::string> &scopes, Dwarf_Error *error) {
        function func;
        char* name = nullptr;
        if (dwarf_diename(die, &name, error) == DW_DLV_OK) {
            func.name = name;
        }

        func.namespace_ = getNamespace(scopes);
        func.mangled = getMangled(die, error);

        if (!func.name.empty()) {
            Dwarf_Off dieOffset = 0;

            if (dwarf_dieoffset(die, &dieOffset, error) == DW_DLV_OK) {
                subprogramInfoByOffset[dieOffset] = { func.name, func.mangled, func.namespace_ };
            }
        }

        Dwarf_Addr startAddress = 0;
        Dwarf_Addr highAddress = 0;
        Dwarf_Half highPcForm = 0;
        Dwarf_Form_Class highPcClass = DW_FORM_CLASS_UNKNOWN;

        if (dwarf_lowpc(die, &startAddress, error) != DW_DLV_OK) return;
        if (dwarf_highpc_b(die, &highAddress, &highPcForm, &highPcClass, error) != DW_DLV_OK) return;

        func.startAddr = static_cast<unsigned long>(startAddress);

        if (highPcClass == DW_FORM_CLASS_CONSTANT) {
            func.size = static_cast<unsigned long>(highAddress);
        } else if (highPcClass == DW_FORM_CLASS_ADDRESS) {
            func.size = static_cast<unsigned long>(highAddress - startAddress);
        } else {
            return; // glup
        }

        Dwarf_Off referenceOffset = 0;

        bool hasReference = getReferenceOffset(die, DW_AT_specification, referenceOffset, error);

        if (!hasReference) {
            hasReference = getReferenceOffset(die, DW_AT_abstract_origin,referenceOffset, error);
        }

        pendingFunctions.push_back({func, referenceOffset, hasReference });
    }

    void inspectDie(Dwarf_Debug dbg, Dwarf_Die die, std::vector<std::string>& scopes, Dwarf_Error* error) {
        Dwarf_Half tag = 0;

	    if (dwarf_tag(die, &tag, error) != DW_DLV_OK) {
		    return;
	    }

        bool pushedScope = false;

        if (isScopeTag(tag)) {
            char* name = nullptr;

            if (dwarf_diename(die, &name, error) == DW_DLV_OK) {
                scopes.emplace_back(name);
                pushedScope = true;
            }
        }

        if (tag == DW_TAG_subprogram) {
            inspectFunction(die, scopes, error);
        }

        Dwarf_Die child = nullptr;

        if (int childErr = dwarf_child(die, &child, error); childErr != DW_DLV_OK) {
            if (childErr != DW_DLV_NO_ENTRY) {
                printf("child not ok: %d\n", childErr);
            }

            if (pushedScope) {
                scopes.pop_back();
            }
            return;
        }

        while (true) {
            inspectDie(dbg, child, scopes, error);

            Dwarf_Die sibling = nullptr;
            int err = dwarf_siblingof_c(child, &sibling, error);

            dwarf_dealloc_die(child);

            if (err != DW_DLV_OK) {
                break;
            }

            child = sibling;
        }

        if (pushedScope) {
            scopes.pop_back();
        }
    }

    bool readDwarf() {
        fatInitDefault();

        Dwarf_Debug dbg = nullptr;
        Dwarf_Error error = nullptr;

        int result = dwarf_init_path(dwarf_path, nullptr, 0, DW_GROUPNUMBER_ANY, nullptr, nullptr, &dbg, &error);

        if (result != DW_DLV_OK) {
            printf("Failed to open DWARF file: %s\n",
                error ? dwarf_errmsg(error) : "No DWARF data found");

            if (error) {
                dwarf_dealloc_error(dbg, error);
            }

            return false;
        }

        while (true) {
            Dwarf_Die cuDie = nullptr;
            Dwarf_Unsigned headerLength = 0;
            Dwarf_Half version = 0;
            Dwarf_Off abbrevOffset = 0;
            Dwarf_Half addressSize = 0;
            Dwarf_Half offsetSize = 0;
            Dwarf_Half extensionSize = 0;
            Dwarf_Sig8 signature{};
            Dwarf_Unsigned typeOffset = 0;
            Dwarf_Unsigned nextHeader = 0;
            Dwarf_Half unitType = 0;

            result = dwarf_next_cu_header_e(
                dbg, true, &cuDie, &headerLength, &version,
                &abbrevOffset, &addressSize, &offsetSize,
                &extensionSize, &signature, &typeOffset,
                &nextHeader, &unitType, &error
            );

            if (result == DW_DLV_NO_ENTRY) {
                break;
            }

            if (result == DW_DLV_ERROR) {
                printf("DWARF parsing failed: %s\n", dwarf_errmsg(error));
                dwarf_dealloc_error(dbg, error);
                error = nullptr;

                dwarf_finish(dbg);

                return false;
            }


            std::vector<std::string> scopes;
            inspectDie(dbg, cuDie, scopes, &error);
            dwarf_dealloc_die(cuDie);
        }

        for (const pending_function& pending : pendingFunctions) {
            function func = pending.implementation;

            if (pending.hasReference) {
                auto it = subprogramInfoByOffset.find(pending.referenceOffset);

                if (it != subprogramInfoByOffset.end()) {
                    func.name = it->second.name;
                    func.mangled = it->second.mangled;
                    func.namespace_ = it->second.namespace_;
                }
            }

            if (func.name.empty()) {
                continue;
            }

            functions.push_back(func);
            addr2funcMap[func.startAddr] = func;
        }

        dwarf_finish(dbg);

        return true;
    }

    function getFunctionByMangled(std::string mangled) {
        for (function func : functions) {
            if (func.mangled == mangled) {
                return func;
            }
        }
        return EMPTY_FUNC;
    }

    std::vector<function> getFunctionByName(std::string namespace_name) {
        std::vector<function> funcs;

        for (const function& func : functions) {
            if (func.name + func.namespace_ == namespace_name) {
                funcs.push_back(func);
            }
        }

        return funcs;
    }

    std::vector<function> getFunctionByName(std::string namespace_, std::string name) {
        std::vector<function> funcs;
        for (function func : functions) {
            if (func.name == name && func.namespace_ == namespace_) {
                funcs.push_back(func);
            }
        }

        return funcs;
    }

    bool applyPatches() {
        function test = getFunctionByName("test")[0];

        auto* start = reinterpret_cast<short *>(test.startPointer());
        start[3] = 69;

        return true;
    }

    void readDebugInfo() {
        if (!readDwarf()) {
            printf("DWARF failed to read, aborting.");
            return;
        }

        if (!applyPatches()) {
            printf("Patches and injects not applied correctly. Proceed with caution!");

            // todo: maybe save a flag for next launch to not launch with patches
        }

#define printfunc(func) printf("namespace: %s, name: %s\n\tstart: %lx, size: %lx mangled: %s\n", func.namespace_.c_str(), func.name.c_str(), func.startAddr, func.size, func.mangled.c_str())

        function func = getFunctionByName("kr1v", "evil")[0];
        printfunc(func);

        func = getFunctionByMangled("_Z4evilv");
        printfunc(func);

        func = getFunctionByName("kr1v::dwarf", "readDebugInfo")[0];
        printfunc(func);
    }
}
