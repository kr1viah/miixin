cmake_minimum_required(VERSION 3.20)
project(DwarfWii LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dwarf-wii
        src/main.cpp
)

set_source_files_properties(
        src/dwarf.cpp
        src/evil.cpp
        PROPERTIES HEADER_FILE_ONLY TRUE
)

target_sources(dwarf-wii PRIVATE
        src/dwarf.cpp
        src/evil.cpp
)

target_link_libraries(dwarf-wii PRIVATE fat)

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

set(DOLPHIN_SD_SYNC_DIR "${CMAKE_SOURCE_DIR}/dolphin-sd"
        CACHE PATH "Dolphin SD sync folder")

add_custom_command(TARGET dwarf-wii POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${DOLPHIN_SD_SYNC_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:dwarf-wii>"
        "${DOLPHIN_SD_SYNC_DIR}/dwarf/main.dwarf"
        COMMENT "Copying DWARF to Dolphin SD"
        VERBATIM
)

find_program(FLATPAK_EXECUTABLE flatpak REQUIRED)

add_custom_target(run-dolphin
        COMMAND "${FLATPAK_EXECUTABLE}" run
        org.DolphinEmu.dolphin-emu
        "--exec=${DOL_FILE}"
        DEPENDS dwarf-wii
        USES_TERMINAL
        VERBATIM
)