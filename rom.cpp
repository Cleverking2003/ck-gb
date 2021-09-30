#include "rom.hpp"

ROM::ROM(const char* game) {
    File file;
    file.open(game, "rb");
    m_size = file.size();
    m_rom = new unsigned char[m_size];
    file.read((char*)m_rom, m_size);
    file.close();
}

//TODO mappers
unsigned char ROM::read8(int addr) {
    return m_rom[addr];
}
