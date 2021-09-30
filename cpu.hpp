#pragma once

class Emulator;

class CPU {
public:
    CPU(Emulator* emul) : m_emul(emul) {}
    bool exec();
private:
    bool exec_cb();

    void inc8(unsigned char& val);
    void inc16(unsigned short& val);
    void dec8(unsigned char& val);
    void dec16(unsigned short& val);
    void add8(unsigned char& val, unsigned char& val2);
    void add16(unsigned short& val, unsigned short& val2);
    void swap(unsigned char& val);

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
