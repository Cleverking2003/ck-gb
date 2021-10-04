#pragma once

#include "file.hpp"

class ROM {
public:
    bool load(const char* game);
    void unload() { delete m_rom; }
    unsigned char read8(int addr);
    //TODO mappers
    void write8(int addr, unsigned char val) {};
    ~ROM() { unload(); }
private:
    unsigned char* m_rom { nullptr };
    int m_size { 0 };
};
