#include "rom.hpp"
#include <iostream>

// TODO: exram
void ROM::mbc1_write(unsigned short addr, unsigned char val) {
    switch (addr) {
    case 0x0000 ... 0x1fff:
        break;
    case 0x2000 ... 0x3fff:
        m_current_bank = val & 0x1f;
        if (m_current_bank == 0) m_current_bank++;
        break;
    case 0x4000 ... 0x5fff:
        if (!m_mode_select) {
            m_current_bank &= 0x1f;
            m_current_bank |= (val & 3) << 5;
        }
        break;
    case 0x6000 ... 0x7fff:
        m_mode_select = val;
        break;
    }
}

bool ROM::load(const char* game) {
    File file;
    if (!file.open(game, "rb")) {
        return false;
    }
    m_size = file.size();
    m_rom = new unsigned char[m_size];
    file.read((char*)m_rom, m_size);
    file.close();
    m_mapper_type = m_rom[0x147];
    if (m_mapper_type != 0 && m_mapper_type != 1) {
        std::cout << "Mapper type " << m_mapper_type << " is unsupported\n";
        return false;
    }
    return true;
}

unsigned char ROM::read8(unsigned short addr) {
    switch (addr) {
    case 0x0000 ... 0x3fff:
        return m_rom[addr];
    case 0x4000 ... 0x7fff:
        return m_rom[addr + (m_current_bank - 1) * 0x4000];
    }
}

void ROM::write8(unsigned short addr, unsigned char val) {
    switch (m_mapper_type) {
    case 0x00: break;
    case 0x01:
        mbc1_write(addr, val);
        break;
    }
}
