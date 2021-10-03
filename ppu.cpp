#include "emul.hpp"
#include "ppu.hpp"
#include <iostream>

unsigned char PPU::read8(int addr) {
    switch(addr) {
    case 0xff41:
        return m_stat.stat;
    case 0xff44:
        return m_ly;
    default:
        std::cout << "Unimplemented read from " << std::hex << addr << '\n';
        return 0;
    }
}

void PPU::write8(int addr, unsigned char val) {
    switch(addr) {
    case 0xff41: {
        auto old_stat = m_stat.stat;
        m_stat.stat = (val & 0xf8) | (old_stat & 0x7);
        break;
    }
    case 0xff44:
        m_ly = 0;
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
            m_emul->raise_int(0);
        }
    }
    else if (m_scanline_cycles < 80) {
        m_stat.mode = 2;
    }
    else if (m_scanline_cycles < (80 + 172)) {
        m_stat.mode = 3;
    }
    else {
        m_stat.mode = 0;
    }
}
