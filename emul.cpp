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

unsigned char Emulator::read8(unsigned short addr) {
    switch (addr) {
    case 0x0000 ... 0x7fff:
    case 0xa000 ... 0xbfff:
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
    case 0xff04 ... 0xff07:
        return s_the->m_timer.read8(addr);
    case 0xff0f:
        return s_the->m_if;
    case 0xff40 ... 0xff4b:
        return s_the->m_ppu.read8(addr);
    case 0xff80 ... 0xfffe:
        return s_the->m_hram[addr - 0xff80];
    case 0xffff:
        return s_the->m_ie;
    default:
        return 0;
    }
}

unsigned short Emulator::read16(unsigned short addr) {
    return (read8(addr + 1) << 8) | read8(addr);
}

void Emulator::write8(unsigned short addr, unsigned char val) {
    switch (addr) {
    case 0x0000 ... 0x7fff:
    case 0xa000 ... 0xbfff:
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
    case 0xff04 ... 0xff07:
        s_the->m_timer.write8(addr, val);
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
        break;
    }
}

void Emulator::write16(unsigned short addr, unsigned short val) {
    write8(addr, val & 0xff);
    write8(addr + 1, val >> 8);
}

void Emulator::exec() {
    for (int i = 0; i < 5; i++) {
        if (s_the->m_if & (1 << i)) {
            s_the->raise_int(i);
            s_the->m_cpu.set_running(true);
            break;
        }
    }

    if (!s_the->m_cpu.running()) {
        s_the->m_ppu.exec(4);
        s_the->m_timer.exec(4);
        return;
    }

    int cycles = s_the->m_cpu.exec();
    s_the->m_elapsed_cycles += cycles;
    s_the->m_ppu.exec(cycles);
    s_the->m_timer.exec(cycles);
}

void Emulator::draw(sf::Sprite& screen) {
        sf::Event e;
        while(s_the->m_window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                s_the->m_window.close();
                s_the->m_running = false;
                return;
            }
        }
        s_the->m_window.clear(sf::Color::White);
        s_the->m_window.draw(screen);
        s_the->m_window.display();
        sf::sleep(sf::milliseconds(16));
}

void Emulator::run() {
    while (s_the->m_running) {
        Emulator::exec();
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
        s_the->m_cpu.jump_to(0x40 | (interrupt << 3));
    }
}

void Emulator::request_int(int interrupt) {
    s_the->m_if |= (1 << interrupt);
} 
