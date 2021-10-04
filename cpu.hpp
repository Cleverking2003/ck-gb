#pragma once

class CPU {
public:
    int exec();
    void jump_to(unsigned short addr) { push(pc); pc = addr; }
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
    void res(unsigned char&, unsigned char);
    void set(unsigned char&, unsigned char);
    void push(unsigned short);
    void pop(unsigned short&);
    void swap(unsigned char&);
    void sla(unsigned char&);
    void srl(unsigned char&);
    void bit(unsigned char, unsigned char);

    unsigned short pc { 0x100 };
    unsigned short sp { 0 };
    struct {
        union {
            unsigned short af;
            struct {
                struct {
                    bool z : 1;
                    bool n : 1;
                    bool h : 1;
                    bool c : 1;
                    int pad : 4;
                } flags;
                unsigned char a;
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
};
