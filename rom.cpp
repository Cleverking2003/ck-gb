#include "rom.hpp"

bool ROM::load(const char* game) {
    File file;
    if (!file.open(game, "rb")) {
        return false;
    }
    m_size = file.size();
    m_rom = new unsigned char[m_size];
    file.read((char*)m_rom, m_size);
    file.close();
    return true;
}

//TODO mappers
unsigned char ROM::read8(int addr) {
    return m_rom[addr];
}
