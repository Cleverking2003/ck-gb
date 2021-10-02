cmake_minimum_required(VERSION 3.10)

project(ck-gb)

add_executable(ck-gb cpu.cpp emul.cpp file.cpp main.cpp ppu.cpp rom.cpp)