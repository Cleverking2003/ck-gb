#include "emul.hpp"
#include "ppu.hpp"
#include <iostream>

unsigned char PPU::read8(unsigned short addr) {
    switch(addr) {
    case 0x8000 ... 0x97ff:
        return m_vram_tiles[addr - 0x8000];
    case 0x9800 ... 0x9bff:
        return m_vram_bg_map1[addr - 0x9800];
    case 0x9c00 ... 0x9fff:
        return m_vram_bg_map2[addr - 0x9c00];
    case 0xfe00 ... 0xfe9f:
        if (!(m_lcdc & 0x80) || !(m_stat & 0x2))
            return m_oam[addr - 0xfe00];
        else
            return 0xff;
    case 0xff40:
        return m_lcdc;
    case 0xff41:
        return m_stat;
    case 0xff42:
        return m_scy;
    case 0xff43:
        return m_scx;
    case 0xff44:
        return m_ly;
    case 0xff45:
        return m_lyc;
    case 0xff46:
        return 0;
    case 0xff47:
        return m_bgp;
    case 0xff48:
        return m_obp0;
    case 0xff49:
        return m_obp1;
    case 0xff4a:
        return m_wy;
    case 0xff4b:
        return m_wx;
    default:
        std::cout << "Unimplemented read from " << std::hex << addr << '\n';
        return 0;
    }
}

void PPU::write8(unsigned short addr, unsigned char val) {
    switch(addr) {
    case 0x8000 ... 0x97ff:
        if (!(m_lcdc & 0x80) || (m_stat & 0x3) != 0x3)
            m_vram_tiles[addr - 0x8000] = val;
        break;
    case 0x9800 ... 0x9bff:
        if (!(m_lcdc & 0x80) || (m_stat & 0x3) != 0x3)
            m_vram_bg_map1[addr - 0x9800] = val;
        break;
    case 0x9c00 ... 0x9fff:
        if (!(m_lcdc & 0x80) || (m_stat & 0x3) != 0x3)
            m_vram_bg_map2[addr - 0x9c00] = val;
        break;
    case 0xfe00 ... 0xfe9f:
        if (!(m_lcdc & 0x80) || !(m_stat & 0x2))
            m_oam[addr - 0xfe00] = val;
        break;
    case 0xff40:
        m_lcdc = val;
        break;
    case 0xff41: {
        auto old_stat = m_stat;
        m_stat = (val & 0xf8) | (old_stat & 0x7);
        break;
    }
    case 0xff42:
        m_scy = val;
        break;
    case 0xff43:
        m_scx = val;
        break;
    case 0xff44:
        m_ly = 0;
        break;
    case 0xff45:
        m_lyc = val;
        break;
    case 0xff46:
        for (int i = 0; i < 0xa0; i++) {
            m_oam[i] = Emulator::read8((val << 8) + i);
        }
        break;
    case 0xff47:
        m_bgp = val;
        break;
    case 0xff48:
        m_obp0 = val;
        break;
    case 0xff49:
        m_obp1 = val;
        break;
    case 0xff4a:
        m_wy = val;
        break;
    case 0xff4b:
        m_wx = val;
        break;
    default:
        std::cout << "Unimplemented write to " << std::hex << addr << '\n';
        break;
    }
}

void PPU::exec(int cycles) {
    m_scanline_cycles += cycles;
    if (m_scanline_cycles > 456) {
        m_ly++;
        m_ly %= 154;
        m_scanline_cycles %= 456;
        if (m_ly == m_lyc) {
            m_stat |= 0x4;
            if (m_stat & 0x40)
                Emulator::request_int(1);
        }
        else {
            m_stat &= ~0x4;
        }
    }
    int mode = m_stat & 3;
    if (m_ly >= 144) {
        if (mode != 1) {
            mode = 1;
            m_stat &= ~3;
            m_stat |= 1;

            for (int y = 0; y < 144; y++) {
                for (int x = 0; x < 160; x++) {
                    m_pixels[(y*160+x)*4] = m_pixels[(y*160+x)*4+1] = m_pixels[(y*160+x)*4+2] = (3 - m_screen[y][x]) * 85;
                    m_pixels[(y*160+x)*4+3] = 255;
                }
            }
            m_frame.create(160, 144, m_pixels);
            m_frame_texture.loadFromImage(m_frame);
            m_frame_sprite.setTexture(m_frame_texture);
            Emulator::draw(m_frame_sprite);

            Emulator::request_int(0);
            if (m_stat & 0x10)
                Emulator::request_int(1);
            frames++;
        }
    }
    else if (m_scanline_cycles < 80) {
        if (mode != 2) {
            m_stat &= ~3;
            m_stat |= 2;
            if (m_stat & 0x20)
                Emulator::request_int(1);
        }
    }
    else if (m_scanline_cycles < (80 + 172)) {
        m_stat |= 3;
    }
    else {
        if (mode != 0) {
            m_stat &= ~3;
            draw_line();
            if (m_stat & 0x8)
                Emulator::request_int(1);
        }
    }
}

void PPU::draw_line() {
    if (!(m_lcdc & 0x80)) {
        return;
    }
    if (m_lcdc & 1) {
        int bg_tiles_base = (m_lcdc & 0x10) ? 0x000 : 0x1000;
        int end_x;
        if ((m_ly >= m_wy) && (m_lcdc & 0x20))
            end_x = m_wx - 7;
        else
            end_x = 160;
        unsigned char cur_x = m_scx;
        unsigned char line = m_scy + m_ly;
        unsigned char* map = (m_lcdc & 8) ? m_vram_bg_map2 : m_vram_bg_map1;
        for (int x = 0; x < 160; x++) {
            if (x == end_x) {
                line = m_ly - m_wy;
                cur_x = x - (m_wx - 7);
                map = (m_lcdc & 0x40) ? m_vram_bg_map2 : m_vram_bg_map1;
            }
            int tile_idx = (line >> 3 << 5) | (cur_x >> 3);
            int tile = map[tile_idx];
            if (!(m_lcdc & 0x10)) tile = (signed char) tile;
            int start_byte = tile * 16 + ((line & 7) << 1);
            auto data = m_vram_tiles[bg_tiles_base + start_byte], 
                data2 = m_vram_tiles[bg_tiles_base + start_byte + 1];
            auto bit_idx = 7 - (cur_x & 0x7);
            auto color_idx = (((data2 >> bit_idx) & 1) << 1) | ((data >> bit_idx) & 1);
            auto color = (m_bgp >> (color_idx * 2)) & 3;
            m_screen[m_ly][x] = color;
            cur_x++;
        }
    }
    if (m_lcdc & 2)
        draw_sprites();
}

void PPU::draw_sprites() {
    auto tile_height = (m_lcdc & 0x4) ? 16 : 8;
    char sprites[10] = { 0 };
    int sprites_x[10] = { 0 };
    int num_sprites = 0;
    for (int i = 0; i < 40; i++) {
        int y = m_oam[i*4];
        if (y == 0 || y >= 160) continue;
        y -= 16;
        if (y > m_ly || (y + tile_height) <= m_ly) continue;
        sprites[num_sprites] = i;
        sprites_x[num_sprites] = m_oam[i*4+1] - 8;
        if (++num_sprites == 10) break;
    }
    for (int x = 0; x < 160; x++) {
        int previous_x = 256;
        int previous = 11;
        for (int i = 0; i < num_sprites; i++) {
            if (sprites_x[i] > x || (sprites_x[i] + 7) < x) continue;
            if ((sprites_x[i] > previous_x) || (sprites_x[i] == previous_x && i > previous)) continue;
            int sc_x = x - sprites_x[i];
            int sprite_idx = sprites[i];
            auto attr = m_oam[sprite_idx*4+3];
            bool x_flip = attr & 0x20, y_flip = attr & 0x40;
            auto pal = (attr & 0x10) ? m_obp1 : m_obp0;
            int y = m_oam[sprite_idx*4] - 16;
            unsigned char tile;
            if (tile_height == 8) {
                tile = m_oam[sprite_idx*4+2];
            }
            else {
                if (m_ly > (y + 7)) tile = m_oam[sprite_idx*4+2] | 1;
                else tile = m_oam[sprite_idx*4+2] & 0xfe;
            }
            auto y_index = (m_ly - y) % 8 * 2;
            if (y_flip) {
                y_index = 14 - y_index;
                if (tile_height == 16) {
                    tile ^= 1;
                }
            }
            auto data = m_vram_tiles[tile*16+y_index],
                data2 = m_vram_tiles[tile*16+y_index + 1];
            auto bit_idx = (x_flip) ? sc_x % 8 : 7 - (sc_x % 8);
            auto color_idx = ((data >> bit_idx) & 1) | (((data2 >> bit_idx) & 1) << 1);
            if (color_idx == 0) continue;
            previous_x = sprites_x[i];
            previous = i;
            auto color = (pal >> (color_idx * 2)) & 3;
            if (m_screen[m_ly][x] == 0 || !(attr & 0x80))
                m_screen[m_ly][x] = color;
        }
    }
}
