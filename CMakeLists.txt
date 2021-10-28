cmake_minimum_required(VERSION 3.10)

project(ck-gb)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
add_executable(ck-gb cpu.cpp emul.cpp file.cpp main.cpp ppu.cpp rom.cpp timer.cpp)
target_link_libraries(ck-gb sfml-graphics sfml-window sfml-system)
