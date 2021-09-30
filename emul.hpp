#pragma once

#include "cpu.hpp"
#include "rom.hpp"

class Emulator {
public:
    Emulator(const char* game) : m_rom(game), m_cpu(this) {}
    unsigned char read8(int addr);
    unsigned short read16(int addr);
    void write8(int addr, unsigned char val);
    void write16(int addr, unsigned short val);
    bool exec();
private:
    ROM m_rom;
    CPU m_cpu;
    unsigned char m_wram[0x2000] { 0 };
    unsigned char m_hram[0x7f] { 0 };
};
