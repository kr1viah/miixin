cmake_minimum_required(VERSION 3.20)
project(DwarfWii LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dwarf-wii
        src/main.cpp
        src/dwarf.cpp
        src/dwarf.h
        src/mixin.h
)

add_executable(plugin
        plugin/main.cpp
)

# evil cross compiling (its bad)
set(BUILD_MMAP OFF CACHE BOOL "" FORCE)
set(ENABLE_DECOMPRESSION OFF CACHE BOOL "" FORCE)
set(BUILD_DWARFDUMP OFF CACHE BOOL "" FORCE)
set(BUILD_DWARFGEN OFF CACHE BOOL "" FORCE)
set(BUILD_DWARFEXAMPLE OFF CACHE BOOL "" FORCE)
set(DO_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/libdwarf EXCLUDE_FROM_ALL)

set_target_properties(dwarf-shared PROPERTIES
        OUTPUT_NAME "dwarf-shared-wii"
)

target_link_libraries(dwarf-wii PRIVATE dwarf-static)
# end of evil cross compiling

set_source_files_properties(
        src/evil.cpp
        PROPERTIES HEADER_FILE_ONLY TRUE
)

target_sources(dwarf-wii PRIVATE
        src/evil.cpp
)

target_link_libraries(dwarf-wii PRIVATE fat)

target_compile_options(dwarf-wii PRIVATE
        -g
        -gdwarf-4
)

target_compile_options(plugin PRIVATE
        -g
        -gdwarf-4
)

target_link_options(plugin PRIVATE
        -Wl,--no-gc-sections
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

add_custom_command(TARGET dwarf-wii POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${DOLPHIN_SD_SYNC_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:plugin>"
        "${DOLPHIN_SD_SYNC_DIR}/plugins/plugin.elf"
        COMMENT "Copying test plugin to Dolphin SD"
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