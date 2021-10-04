#pragma once

class PPU {
public:
    unsigned char read8(int addr);
    void write8(int addr, unsigned char val);
    void exec(int cycles);
private:
    unsigned char m_vram_tiles1[0x1000] { 0 };
    unsigned char m_vram_tiles2[0x800] { 0 };
    unsigned char m_vram_bg_map1[0x400] { 0 };
    unsigned char m_vram_bg_map2[0x400] { 0 };
    unsigned char m_oam[0xa0] { 0 };
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
