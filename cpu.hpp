#pragma once

class CPU {
public:
    int exec();
    void jump_to(unsigned short addr) { push(pc); pc = addr; }
    bool running() { return m_running; }
    void set_running(bool b) { m_running = b; }
private:
    int exec_cb();

    void inc(unsigned char&);
    void dec(unsigned char&);
    void add(unsigned char&, unsigned char);
    void adc(unsigned char&, unsigned char);
    void sub(unsigned char&, unsigned char);
    void sbc(unsigned char&, unsigned char);
    void log_and(unsigned char);
    void log_or(unsigned char);
    void log_xor(unsigned char);
    void cp(unsigned char);
    void rlc(unsigned char&);
    void rrc(unsigned char&);
    void rl(unsigned char&);
    void rr(unsigned char&);
    void res(unsigned char&, unsigned char);
    void set(unsigned char&, unsigned char);
    void push(unsigned short);
    void pop(unsigned short&);
    void swap(unsigned char&);
    void sla(unsigned char&);
    void srl(unsigned char&);
    void bit(unsigned char, unsigned char);
    bool getZ() { return regs.f & 0x80; };
    void setZ(bool val) { 
        if (val) regs.f |= 0x80;
        else regs.f &= ~0x80;
    }
    bool getN() { return regs.f & 0x40; };
    void setN(bool val) { 
        if (val) regs.f |= 0x40;
        else regs.f &= ~0x40;
    }
    bool getH() { return regs.f & 0x20; };
    void setH(bool val) { 
        if (val) regs.f |= 0x20;
        else regs.f &= ~0x20;
    }
    bool getC() { return regs.f & 0x10; };
    void setC(bool val) { 
        if (val) regs.f |= 0x10;
        else regs.f &= ~0x10;
    }

    unsigned short pc { 0x100 };
    unsigned short sp { 0 };
    struct {
        union {
            unsigned short af;
            struct {
                unsigned char f, a;
            };
        };
        union {
            unsigned short bc;
            struct {
                unsigned char c, b;
            };
        };
        union {
            unsigned short de;
            struct {
                unsigned char e, d;
            };
        };
        union {
            unsigned short hl;
            struct {
                unsigned char l, h;
            };
        };
    } regs { 0 };
    bool m_running { true };
};
