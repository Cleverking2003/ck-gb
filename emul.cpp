#include "emul.hpp"
#include <iostream>

static Emulator* s_the;

Emulator* Emulator::create() {
    if (!s_the)
        s_the = new Emulator;
    return s_the;
}

bool Emulator::load(const char* game) { 
    s_the->m_window.create(sf::VideoMode(160, 144), "CleverKing's Gameboy Emulator", sf::Style::Default);
    return s_the->m_rom.load(game);
}
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
        if (!(s_the->m_joyp & 0x20)) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) s_the->m_joyp &= ~0x8;
            else s_the->m_joyp |= 0x8;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) s_the->m_joyp &= ~0x4;
            else s_the->m_joyp |= 0x4;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) s_the->m_joyp &= ~0x2;
            else s_the->m_joyp |= 0x2;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) s_the->m_joyp &= ~0x1;
            else s_the->m_joyp |= 0x1;
        }
        else if (!(s_the->m_joyp & 0x10)) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) s_the->m_joyp &= ~0x8;
            else s_the->m_joyp |= 0x8;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) s_the->m_joyp &= ~0x4;
            else s_the->m_joyp |= 0x4;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) s_the->m_joyp &= ~0x2;
            else s_the->m_joyp |= 0x2;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) s_the->m_joyp &= ~0x1;
            else s_the->m_joyp |= 0x1;
        }
        return s_the->m_joyp;
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
    case 0xff00:
        s_the->m_joyp = val | 0xf;
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
    s_the->m_elapsed_cycles += cycles;
    s_the->m_ppu.exec(cycles);
    return cycles != 0;
}

void Emulator::run() {
    while (Emulator::exec()) {
        if (s_the->m_elapsed_cycles > 4194304 / 60) {
            s_the->m_elapsed_cycles -= 4194304 / 60;
            sf::Event e;
            while(s_the->m_window.pollEvent(e)) {
                if (e.type == sf::Event::Closed) {
                    s_the->m_window.close();
                    return;
                }
            }
            s_the->m_window.clear(sf::Color::White);
            s_the->m_window.draw(*s_the->m_ppu.build_image());
            s_the->m_window.display();
            sf::sleep(sf::milliseconds(16));
        }
    }
}

// 0 = VBlank
// 1 = LCD stat
// 2 = Timer
// 3 = Serial
// 4 = Joypad
void Emulator::raise_int(int interrupt) {
    if (s_the->m_ie & (1 << interrupt) && s_the->m_interrupts_enabled) {
        s_the->m_interrupts_enabled = false;
        s_the->m_if &= ~(1 << interrupt);
        s_the->m_cpu.jump_to(0x40 + interrupt * 8);
    }
    else {
        s_the->m_if |= (1 << interrupt);
    }
}
