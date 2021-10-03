#pragma once

class Emulator;

class PPU {
public:
    PPU(Emulator* emul) : m_emul(emul) {}
    unsigned char read8(int addr);
    void write8(int addr, unsigned char val);
    void exec(int cycles);
private:
    Emulator* m_emul;
    union {
        unsigned char stat;
        struct {
            bool : 1;
            bool lyc : 1;
            bool oam_int : 1;
            bool vbla_int : 1;
            bool hbla_int : 1;
            bool coincidence : 1;
            char mode : 2;
        };
    } m_stat { 0 };
    int m_ly { 0 };
    int m_scanline_cycles { 0 };
};
