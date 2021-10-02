#include "emul.hpp"
#include "ppu.hpp"

void PPU::exec(int cycles) {
    m_scanline_cycles += cycles;
    if (m_scanline_cycles > 456) {
        m_ly++;
        m_ly %= 154;
        m_scanline_cycles %= 456;
    }
    if (m_ly >= 144) {
        m_stat.mode = 1;
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
