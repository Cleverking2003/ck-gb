#pragma once

#include "cpu.hpp"
#include "ppu.hpp"
#include "rom.hpp"

class Emulator {
public:
    static Emulator* the();
    static bool load(const char* game);
    static void unload();
    static unsigned char read8(int addr);
    static unsigned short read16(int addr);
    static void write8(int addr, unsigned char val);
    static void write16(int addr, unsigned short val);
    static void raise_int(int interrupt);
    static void enable_ints();
    static void disable_ints();
    static bool exec();
private:
    Emulator() {}
    Emulator(const Emulator&) = delete;
    Emulator(const Emulator&&) = delete;
    int operator=(const Emulator&) = delete;
    int operator=(const Emulator&&) = delete;
    ROM m_rom;
    CPU m_cpu;
    PPU m_ppu;
    unsigned char m_wram[0x2000] { 0 };
    unsigned char m_hram[0x7f] { 0 };
    unsigned char m_if { 0 }, m_ie { 0 };
    bool m_interrupts_enabled { true };
};
