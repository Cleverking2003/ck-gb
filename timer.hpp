#pragma once

class Timer {
public:
    unsigned char read8(int addr);
    void write8(int addr, unsigned char val);
    void exec(int cycles);
private:
    unsigned char m_div { 0 }, m_tima { 0 }, m_tma { 0 }, m_tac { 0 };
    int m_div_cycles { 0 }, m_tima_cycles { 0 };
};
