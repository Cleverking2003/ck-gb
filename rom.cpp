#include "rom.hpp"
#include <iostream>

// TODO: exram
void ROM::mbc1_write(unsigned short addr, unsigned char val) {
    switch (addr) {
    case 0x0000 ... 0x1fff:
        m_enable_exram = (val & 0xf) == 0xa;
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
        else {
            if (val <= 3) m_exram_bank = val;
        }
        break;
    case 0x6000 ... 0x7fff:
        m_mode_select = val;
        break;
    case 0xa000 ... 0xbfff:
        if (m_enable_exram) {
            m_exram[(m_exram_bank * 0x2000) + (addr - 0xa000)] = val;
        }
        break;
    }
}

// TODO: exram
void ROM::mbc3_write(unsigned short addr, unsigned char val) {
    switch (addr) {
    case 0x0000 ... 0x1fff:
        m_enable_exram = (val & 0xf) == 0xa;
        break;
    case 0x2000 ... 0x3fff:
        m_current_bank = val & 0x7f;
        if (m_current_bank == 0) m_current_bank++;
        break;
    case 0x4000 ... 0x5fff:
        if (val <= 3) m_exram_bank = val;
        break;
    case 0x6000 ... 0x7fff:
        break;
    case 0xa000 ... 0xbfff:
        if (m_enable_exram) {
            m_exram[(m_exram_bank * 0x2000) + (addr - 0xa000)] = val;
        }
        break;
    }
}

// TODO: exram
void ROM::mbc5_write(unsigned short addr, unsigned char val) {
    switch (addr) {
    case 0x0000 ... 0x1fff:
        //TODO
        break;
    case 0x2000 ... 0x2fff:
        m_current_bank &= ~0xff;
        m_current_bank |= val;
        break;
    case 0x3000 ... 0x3fff:
        m_current_bank &= ~0x100;
        m_current_bank |= val & 1;
        break;
    case 0x4000 ... 0x5fff:
        //TODO
        break;
    default:
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
    if (m_mapper_type != 0 && m_mapper_type != 1 && m_mapper_type != 0x13 && m_mapper_type != 0x19) {
        std::cout << "Mapper type 0x" << std::hex << m_mapper_type << " is unsupported\n";
        return false;
    }
    switch (m_rom[0x149]) {
    case 0x02:
        m_exram_size = 1;
        break;
    case 0x03:
        m_exram_size = 4;
        break;
    case 0x04:
        m_exram_size = 16;
        break;
    case 0x05:
        m_exram_size = 8;
        break;
    default:
        m_exram_size = 0;
        break;
    }
    m_exram_size *= 0x2000;
    if (m_exram_size) m_exram = new unsigned char[m_exram_size];
    return true;
}

unsigned char ROM::read8(unsigned short addr) {
    switch (addr) {
    case 0x0000 ... 0x3fff:
        return m_rom[addr];
    case 0x4000 ... 0x7fff:
        return m_rom[addr + (m_current_bank - 1) * 0x4000];
    case 0xa000 ... 0xbfff:
        if (m_enable_exram) {
            return m_exram[(m_exram_bank * 0x2000) + (addr - 0xa000)];
        }
        return 0;
    }
}

void ROM::write8(unsigned short addr, unsigned char val) {
    switch (m_mapper_type) {
    case 0x00: break;
    case 0x01:
        mbc1_write(addr, val);
        break;
    case 0x13:
        mbc3_write(addr, val);
        break;
    case 0x19:
        mbc5_write(addr, val);
        break;
    }
}
