#include "emul.hpp"
#include <iostream>

static Emulator* s_the;

Emulator* Emulator::the() {
    if (!s_the)
        s_the = new Emulator;
    return s_the;
}

bool Emulator::load(const char* game) { return s_the->m_rom.load(game); }
void Emulator::unload() { s_the->m_rom.unload(); }
void Emulator::enable_ints() { s_the->m_interrupts_enabled = true; }
void Emulator::disable_ints() { s_the->m_interrupts_enabled = false; }

unsigned char Emulator::read8(int addr) {
    switch (addr) {
    case 0x0000 ... 0x7fff:
        return s_the->m_rom.read8(addr);
    case 0x8000 ... 0x9fff:
        return s_the->m_ppu.read8(addr);
    case 0xc000 ... 0xdfff:
        return s_the->m_wram[addr - 0xc000];
    case 0xfe00 ... 0xfe9f:
        return s_the->m_ppu.read8(addr);
    case 0xff00:
        return 0x0f;
    case 0xff0f:
        return s_the->m_if;
    case 0xff40 ... 0xff4b:
        return s_the->m_ppu.read8(addr);
    case 0xff80 ... 0xfffe:
        return s_the->m_hram[addr - 0xff80];
    case 0xffff:
        return s_the->m_ie;
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
        s_the->m_rom.write8(addr, val);
        break;
    case 0x8000 ... 0x9fff:
        s_the->m_ppu.write8(addr, val);
        break;
    case 0xc000 ... 0xdfff:
        s_the->m_wram[addr - 0xc000] = val;
        break;
    case 0xfe00 ... 0xfe9f:
        s_the->m_ppu.write8(addr, val);
        break;
    case 0xff0f:
        s_the->m_if = val;
        break;
    case 0xff40 ... 0xff4b:
        s_the->m_ppu.write8(addr, val);
        break;
    case 0xff80 ... 0xfffe:
        s_the->m_hram[addr - 0xff80] = val;
        break;
    case 0xffff:
        s_the->m_ie = val;
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
        if (s_the->m_if & (1 << i)) {
            raise_int(i);
            break;
        }
    }
    int cycles = s_the->m_cpu.exec();
    s_the->m_ppu.exec(cycles);
    return cycles != 0;
}

// 0 = VBlank
// 1 = LCD stat
// 2 = Timer
// 3 = Serial
// 4 = Joypad
void Emulator::raise_int(int interrupt) {
    if (s_the->m_ie & (1 << interrupt) && s_the->m_interrupts_enabled) {
        std::cout << "Executing interrupt " << interrupt << '\n';
        s_the->m_interrupts_enabled = false;
        s_the->m_if &= ~(1 << interrupt);
        s_the->m_cpu.jump_to(0x40 + interrupt * 8);
    }
    else {
        s_the->m_if |= (1 << interrupt);
    }
}
