#include "emul.hpp"
#include <iostream>

unsigned char Emulator::read8(int addr) {
    switch (addr) {
    case 0x0000 ... 0x7fff:
        return m_rom.read8(addr);
    case 0xc000 ... 0xdfff:
        return m_wram[addr - 0xc000];
    case 0xff00:
        return 0x0f;
    case 0xff0f:
        return m_if;
    case 0xff40 ... 0xff4b:
        return m_ppu.read8(addr);
    case 0xff80 ... 0xfffe:
        return m_hram[addr - 0xff80];
    case 0xffff:
        return m_ie;
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
    case 0xff0f:
        m_if = val;
        break;
    case 0xff40 ... 0xff4b:
        m_ppu.write8(addr, val);
        break;
    case 0xff80 ... 0xfffe:
        m_hram[addr - 0xff80] = val;
        break;
    case 0xffff:
        m_ie = val;
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
    for (int i = 0; i < 5; i++) {
        if (m_if & (1 << i)) {
            raise_int(i);
            break;
        }
    }
    int cycles = m_cpu.exec();
    m_ppu.exec(cycles);
    return cycles != 0;
}

// 0 = VBlank
// 1 = LCD stat
// 2 = Timer
// 3 = Serial
// 4 = Joypad
void Emulator::raise_int(int interrupt) {
    if (m_ie & (1 << interrupt) && m_interrupts_enabled) {
        std::cout << "Executing interrupt " << interrupt << '\n';
        m_interrupts_enabled = false;
        m_if &= ~(1 << interrupt);
        m_cpu.jump_to(0x40 + interrupt * 8);
    }
    else {
        m_if |= (1 << interrupt);
    }
}
