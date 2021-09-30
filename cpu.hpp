#pragma once

class Emulator;

class CPU {
public:
    CPU(Emulator* emul) : m_emul(emul) {}
    bool exec();
private:
    bool exec_cb();

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
    void push(unsigned short);
    void pop(unsigned short&);
    void swap(unsigned char&);

    Emulator* m_emul;
    unsigned short pc { 0x100 };
    unsigned short sp { 0 };
    struct {
        union {
            unsigned short af;
            struct {
                unsigned char a;
                struct {
                    bool z : 1;
                    bool n : 1;
                    bool h : 1;
                    bool c : 1;
                    int pad : 4;
                } flags;
            };
        };
        union {
            unsigned short bc;
            struct {
                unsigned char b, c;
            };
        };
        union {
            unsigned short de;
            struct {
                unsigned char d, e;
            };
        };
        union {
            unsigned short hl;
            struct {
                unsigned char h, l;
            };
        };
    } regs { 0 };
    bool m_interrupts_enabled { true };
};
