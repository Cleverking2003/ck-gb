#include "emul.hpp"
#include <iostream>

unsigned char Emulator::read8(int addr) {
    switch (addr) {
    case 0x0000 ... 0x7fff:
        return m_rom.read8(addr);
    case 0xc000 ... 0xdfff:
        return m_wram[addr - 0xc000];
    case 0xff44:
        m_ly++;
        if (m_ly == 154) m_ly = 0;
        return m_ly;
    case 0xff80 ... 0xfffe:
        return m_hram[addr - 0xff80];
    default:
        std::cout << "Unimplemented read from " << std::hex << addr << '\n';
        return 0;
    }
}

unsigned short Emulator::read16(int addr) {
    return (read8(addr + 1) << 8) | read8(addr);
}

void Emulator::write8(int addr, unsigned char val) {
    switch (addr) {
    case 0x0000 ... 0x7fff:
        m_rom.write8(addr, val);
        break;
    case 0xc000 ... 0xdfff:
        m_wram[addr - 0xc000] = val;
        break;
    case 0xff80 ... 0xfffe:
        m_hram[addr - 0xff80] = val;
        break;
    default:
        std::cout << "Unimplemented write to " << std::hex << addr << '\n';
        break;
    }
}

void Emulator::write16(int addr, unsigned short val) {
    write8(addr, val & 0xff);
    write8(addr + 1, val >> 8);
}

bool Emulator::exec() {
    return m_cpu.exec();
}
