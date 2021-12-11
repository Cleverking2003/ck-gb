#pragma once
#include <SFML/Graphics.hpp>

class PPU {
public:
    unsigned char read8(unsigned short addr);
    void write8(unsigned short addr, unsigned char val);
    void exec(int cycles);
private:
    void draw_line();
    void draw_sprites();

    unsigned char m_vram_tiles[0x1800] { 0 };
    unsigned char m_vram_bg_map1[0x400] { 0 };
    unsigned char m_vram_bg_map2[0x400] { 0 };
    unsigned char m_oam[0xa0] { 0 };
    unsigned char m_lcdc { 0x91 };
    unsigned char m_stat { 0 };
    unsigned char m_ly { 0 };
    unsigned char m_lyc { 0 };
    unsigned char m_scy { 0 };
    unsigned char m_scx { 0 };
    unsigned char m_bgp { 0 };
    unsigned char m_obp0 { 0 };
    unsigned char m_obp1 { 0 };
    unsigned char m_wy { 0 };
    unsigned char m_wx { 0 };
    int m_scanline_cycles { 0 };
    unsigned char m_screen[144][160] { {0} };
    sf::Uint8 m_pixels[160 * 144 * 4];
    int frames { 0 };
    sf::Image m_frame;
    sf::Texture m_frame_texture;
    sf::Sprite m_frame_sprite;
};
