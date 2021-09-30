#pragma once

#include "file.hpp"

class ROM {
public:
    ROM(const char* game);
    unsigned char read8(int addr);
    //TODO mappers
    void write8(int addr, unsigned char val) {};
    ~ROM() { delete m_rom; }
private:
    unsigned char* m_rom;
    int m_size;
};
