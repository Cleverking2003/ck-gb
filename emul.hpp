#pragma once

#include "cpu.hpp"
#include "ppu.hpp"
#include "rom.hpp"
#include "timer.hpp"

class Emulator {
public:
    static Emulator* create();
    static bool load(const char* game);
    static void unload();
    static unsigned char read8(unsigned short addr);
    static unsigned short read16(unsigned short addr);
    static void write8(unsigned short addr, unsigned char val);
    static void write16(unsigned short addr, unsigned short val);
    static void raise_int(int interrupt);
    static void enable_ints();
    static void disable_ints();
    static bool exec();
    static void run();
private:
    Emulator() {}
    Emulator(const Emulator&) = delete;
    Emulator(const Emulator&&) = delete;
    int operator=(const Emulator&) = delete;
    int operator=(const Emulator&&) = delete;
    ROM m_rom;
    CPU m_cpu;
    PPU m_ppu;
    Timer m_timer;
    unsigned char m_wram[0x2000] { 0 };
    unsigned char m_hram[0x7f] { 0 };
    unsigned char m_joyp { 0xf }, m_if { 0 }, m_ie { 0 }, m_div { 0 };
    int m_div_cycles { 0 };
    bool m_interrupts_enabled { true };
    int m_elapsed_cycles { 0 };
    sf::RenderWindow m_window;
};
