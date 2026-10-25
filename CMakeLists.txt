cmake_minimum_required(VERSION 3.20)
project(DwarfWii LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dwarf-wii
        src/main.cpp
)

target_compile_options(dwarf-wii PRIVATE
        -g
        -gdwarf-4
)

find_program(ELF2DOL_EXECUTABLE
        NAMES elf2dol
        HINTS "$ENV{DEVKITPRO}/tools/bin"
        REQUIRED
)

set(DOL_FILE "${CMAKE_CURRENT_BINARY_DIR}/dwarf-wii.dol")

add_custom_command(TARGET dwarf-wii POST_BUILD
        COMMAND "${ELF2DOL_EXECUTABLE}"
        "$<TARGET_FILE:dwarf-wii>"
        "${DOL_FILE}"
        BYPRODUCTS "${DOL_FILE}"
        COMMENT "Converting ELF to Wii DOL"
        VERBATIM
)

find_program(DOLPHIN_EXECUTABLE NAMES dolphin-emu Dolphin)

if(DOLPHIN_EXECUTABLE)
    add_custom_target(run-dolphin
            COMMAND "${DOLPHIN_EXECUTABLE}"
            "--exec=${CMAKE_CURRENT_BINARY_DIR}/dwarf-wii.dol"
            DEPENDS dwarf-wii
            USES_TERMINAL
    )
endif()