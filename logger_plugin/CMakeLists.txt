cmake_minimum_required(VERSION 3.20)
project(gta_sa_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force x86 (Win32) build
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "This project must be built as x86 (Win32)")
endif()

# Static CRT linkage — no runtime DLL dependency
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(gta_sa_logger MODULE src/main.cpp src/script.cpp)

# Output as .asi
set_target_properties(gta_sa_logger PROPERTIES
    SUFFIX ".asi"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
)

# No external dependencies — only Windows APIs
target_link_libraries(gta_sa_logger PRIVATE kernel32 winmm)
