cmake_minimum_required(VERSION 3.20)
project(chip8 C)

set(CMAKE_C_STANDARD 11)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
endif()

find_package(raylib REQUIRED)

add_library(chip8_core src/cpu.c)
target_include_directories(chip8_core PUBLIC include)

add_executable(chip8 src/main.c)
target_link_libraries(chip8 PRIVATE chip8_core raylib)

enable_testing()
add_executable(test_cpu tests/test_cpu.c)
target_link_libraries(test_cpu PRIVATE chip8_core)
add_test(NAME CPU_Initialization COMMAND test_cpu)