#include "emul.hpp"
#include "ppu.hpp"
#include <iostream>

unsigned char PPU::read8(int addr) {
    switch(addr) {
    case 0x8000 ... 0x97ff:
        return m_vram_tiles[addr - 0x8000];
    case 0x9800 ... 0x9bff:
        return m_vram_bg_map1[addr - 0x9800];
    case 0x9c00 ... 0x9fff:
        return m_vram_bg_map2[addr - 0x9c00];
    case 0xfe00 ... 0xfe9f:
        return m_oam[addr - 0xfe00];
    case 0xff40:
        return m_lcdc.lcdc;
    case 0xff41:
        return m_stat.stat;
    case 0xff42:
        return m_scy;
    case 0xff43:
        return m_scx;
    case 0xff44:
        return m_ly;
    case 0xff47:
        return m_bgp;
    default:
        std::cout << "Unimplemented read from " << std::hex << addr << '\n';
        return 0;
    }
}

void PPU::write8(int addr, unsigned char val) {
    switch(addr) {
    case 0x8000 ... 0x97ff:
        m_vram_tiles[addr - 0x8000] = val;
        break;
    case 0x9800 ... 0x9bff:
        m_vram_bg_map1[addr - 0x9800] = val;
        break;
    case 0x9c00 ... 0x9fff:
        m_vram_bg_map2[addr - 0x9c00] = val;
        break;
    case 0xfe00 ... 0xfe9f:
        m_oam[addr - 0xfe00] = val;
        break;
    case 0xff40:
        m_lcdc.lcdc = val;
        break;
    case 0xff41: {
        auto old_stat = m_stat.stat;
        m_stat.stat = (val & 0xf8) | (old_stat & 0x7);
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
    case 0xff47:
        m_bgp = val;
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
    }
    if (m_ly >= 144) {
        if (m_stat.mode != 1) {
            m_stat.mode = 1;
            Emulator::raise_int(0);
            if (frames++ == 100) {
                for (int y = 0; y < 144; y++) {
                    for (int x = 0; x < 160; x++) {
                        std::cout << (unsigned)m_screen[y][x];
                    }
                    std::cout << '\n';
                }
                //exit(0);
            }
        }
    }
    else if (m_scanline_cycles < 80) {
        m_stat.mode = 2;
    }
    else if (m_scanline_cycles < (80 + 172)) {
        m_stat.mode = 3;
    }
    else {
        if (m_stat.mode != 0) {
            m_stat.mode = 0;
            draw_line();
        }
    }
}

void PPU::draw_line() {
    if (!(m_lcdc.lcdc & 0x80)) {
        return;
    }
    int bg_tiles_base = (m_lcdc.lcdc & 0x10) ? 0x000 : 0x800;
    auto tile_idx = ((m_scy + m_ly) / 8) * 32 + (m_scx / 8);
    auto tile = (m_lcdc.lcdc & 8) ? m_vram_bg_map2[tile_idx] : m_vram_bg_map1[tile_idx];
    int start_byte = tile * 16 + ((m_scy + m_ly) % 8) * 2;
    if (!(m_lcdc.lcdc & 0x10)) {
        start_byte -= 128 * 16;
    }
    for (int x = 0; x < 160; x++) {
        auto data = m_vram_tiles[bg_tiles_base + start_byte], 
            data2 = m_vram_tiles[bg_tiles_base + start_byte + 1];
        auto bit_idx = 7 - ((m_scx + x) % 8);
        auto color_idx = ((data >> bit_idx) & 1) | (((data2 >> bit_idx) & 1) << 1);
        auto color = (m_bgp >> (color_idx * 2)) & 3;
        m_screen[m_ly][x] = color;
        tile_idx = ((m_scy + m_ly) / 8) * 32 + ((m_scx + x) / 8);
        tile = (m_lcdc.lcdc & 8) ? m_vram_bg_map2[tile_idx] : m_vram_bg_map1[tile_idx];
        start_byte = tile * 16 + ((m_scy + m_ly) % 8) * 2;
        if (!(m_lcdc.lcdc & 0x10)) {
            start_byte -= 128 * 16;
        }
    }
}
