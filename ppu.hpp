#pragma once
#include <SFML/Graphics.hpp>

class PPU {
public:
    unsigned char read8(int addr);
    void write8(int addr, unsigned char val);
    void exec(int cycles);
    void draw_line();
    void draw_sprites();
    sf::Sprite* build_image();
private:
    unsigned char m_vram_tiles[0x1800] { 0 };
    unsigned char m_vram_bg_map1[0x400] { 0 };
    unsigned char m_vram_bg_map2[0x400] { 0 };
    unsigned char m_oam[0xa0] { 0 };
    unsigned char m_lcdc { 0x91 };
    unsigned char m_stat { 0 };
    unsigned char m_ly { 0 };
    unsigned char m_scy { 0 };
    unsigned char m_scx { 0 };
    unsigned char m_bgp { 0 };
    unsigned char m_obp0 { 0 };
    unsigned char m_obp1 { 0 };
    int m_scanline_cycles { 0 };
    unsigned char m_screen[144][160][4] { {0} };
    int frames { 0 };
    sf::Image m_frame;
    sf::Texture m_frame_texture;
    sf::Sprite m_frame_sprite;
};
