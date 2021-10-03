#pragma once

#include "cpu.hpp"
#include "ppu.hpp"
#include "rom.hpp"

class Emulator {
public:
    Emulator(const char* game) : m_rom(game), m_cpu(this), m_ppu(this) {}
    unsigned char read8(int addr);
    unsigned short read16(int addr);
    void write8(int addr, unsigned char val);
    void write16(int addr, unsigned short val);
    void raise_int(int interrupt);
    void enable_ints() { m_interrupts_enabled = true; }
    void disable_ints() { m_interrupts_enabled = false; }
    bool exec();
private:
    ROM m_rom;
    CPU m_cpu;
    PPU m_ppu;
    unsigned char m_wram[0x2000] { 0 };
    unsigned char m_hram[0x7f] { 0 };
    unsigned char m_if { 0 }, m_ie { 0 };
    bool m_interrupts_enabled { true };
};
