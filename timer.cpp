#include "emul.hpp"
#include "timer.hpp"

static const int freq[] = { 4096, 262144, 65536, 16384 };

unsigned char Timer::read8(int addr) {
    switch (addr) {
    case 0xff04:
        return m_div;
    case 0xff05:
        return m_tima;
    case 0xff06:
        return m_tma;
    case 0xff07:
        return m_tac;
    default:
        return 0;
    }
}

void Timer::write8(int addr, unsigned char val) {
    switch (addr) {
    case 0xff04:
        m_div = 0;
        break;
    case 0xff05:
        break;
    case 0xff06:
        m_tma = val;
        break;
    case 0xff07:
        m_tac = val;
        break;
    default:
        break;
    }
}

void Timer::exec(int cycles) {
    m_div_cycles += cycles;
    if (m_div_cycles >= 4194304 / 16384) {
        m_div++;
        m_div_cycles %= 4194304 / 16384;
    }
    if (!(m_tac & 4)) return;
    m_tima_cycles += cycles;
    if (m_tima_cycles >= 4194304 / freq[m_tac & 3]) {
        m_tima++;
        if (m_tima == 0) {
            m_tima = m_tma;
            Emulator::request_int(2);
        }
        m_tima_cycles %= 4194304 / freq[m_tac & 3];
    }
}
