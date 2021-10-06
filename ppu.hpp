#pragma once
#include <SFML/Graphics.hpp>

class PPU {
public:
    unsigned char read8(int addr);
    void write8(int addr, unsigned char val);
    void exec(int cycles);
    void draw_line();
    sf::Sprite* build_image();
private:
    unsigned char m_vram_tiles[0x1800] { 0 };
    unsigned char m_vram_bg_map1[0x400] { 0 };
    unsigned char m_vram_bg_map2[0x400] { 0 };
    unsigned char m_oam[0xa0] { 0 };
    union {
        unsigned char lcdc;
        struct {
            bool display_enable : 1;
            bool window_map_select : 1;
            bool window_enable : 1;
            bool bg_data_select : 1;
            bool bg_map_select : 1;
            bool obj_size : 1;
            bool obj_enable : 1;
            bool bg_enable : 1;
        };
    } m_lcdc;
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
    unsigned char m_ly { 0 };
    unsigned char m_scy { 0 };
    unsigned char m_scx { 0 };
    unsigned char m_bgp { 0 };
    int m_scanline_cycles { 0 };
    unsigned char m_screen[144][160][4] { {0} };
    int frames { 0 };
    sf::Image m_frame;
    sf::Texture m_frame_texture;
    sf::Sprite m_frame_sprite;
};
