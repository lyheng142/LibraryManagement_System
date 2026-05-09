cmake_minimum_required(VERSION 3.14)
project(LibrarySystem)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── FIX: Auto-detect compiler if VS Code doesn't set it ──────────────
if(NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER NAMES gcc cc cl)
endif()
if(NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER NAMES g++ c++ cl)
endif()

# ── FIX: Windows 'byte' ambiguous error ──────────────────────────────
if(WIN32)
    add_compile_definitions(
        _HAS_STD_BYTE=0
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )
endif()

# ── MSVC flags ────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/utf-8 /W1 /wd4996 /wd4267 /wd4244)
endif()

# ── MinGW flags ───────────────────────────────────────────────────────
if(MINGW)
    add_compile_options(
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
        -Wno-deprecated-declarations
    )
    # Fix locale crash on Windows MinGW
    add_compile_definitions(
        _GLIBCXX_USE_C99_STDIO=1
        __USE_MINGW_ANSI_STDIO=1
    )
endif()

# ── OpenXLSX ──────────────────────────────────────────────────────────
set(OPENXLSX_CREATE_DOCS      OFF CACHE BOOL "" FORCE)
set(OPENXLSX_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(OPENXLSX_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(OPENXLSX_BUILD_SAMPLES    OFF CACHE BOOL "" FORCE)
add_subdirectory(external/OpenXLSX)

# ── Executable ────────────────────────────────────────────────────────
add_executable(library
    src/main.cpp
    src/book.cpp
    src/borrow.cpp
    src/excel_helper.cpp
    src/auth.cpp
)

target_include_directories(library PRIVATE
    include
    external/OpenXLSX/OpenXLSX
    external/OpenXLSX/OpenXLSX/headers
    ${CMAKE_BINARY_DIR}/OpenXLSX
)

target_link_libraries(library PRIVATE OpenXLSX)

# ── FIX: Windows extra libs ───────────────────────────────────────────
if(WIN32)
    target_link_libraries(library PRIVATE kernel32)
endif()

# ── FIX: MinGW needs stdc++fs for std::filesystem ────────────────────
if(MINGW)
    target_link_libraries(library PRIVATE stdc++fs)
endif()

# ── Auto-create data/ folder after build ─────────────────────────────
add_custom_command(TARGET library POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/data"
    COMMENT "Creating data/ folder"
)
