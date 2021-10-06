#include "cpu.hpp"
#include "emul.hpp"
#include "ops.hpp"
#include <iostream>

void CPU::inc(unsigned char& val) {
    setZ(++val == 0);
    setN(false);
    setH((val & 0xf) > 9);
}

void CPU::dec(unsigned char& val) {
    setZ(--val == 0);
    setN(true);
    setH((val & 0xf) > 9);
}

void CPU::swap(unsigned char& val) {
    unsigned char lo = val & 0xf, hi = (val >> 4) & 0xf;
    val = (lo << 4) | hi;
    setZ(val == 0);
    setN(false);
    setH(false);
    setC(false);
}

void CPU::add(unsigned char& val, unsigned char val2) {
    setC((val + val2) < max(val, val2));
    val += val2;
    setZ(val == 0);
    setN(false);
    setH((val & 0xf) > 9);
}

void CPU::sub(unsigned char& val, unsigned char val2) {
    setC(val < val2);
    val -= val2;
    setZ(val == 0);
    setN(true);
    setH((val & 0xf) > 9);
}

void CPU::adc(unsigned char& val, unsigned char val2) {
    unsigned char res = val + val2 + (getC() ? 1 : 0);
    setZ(res == 0);
    setN(false);
    setH((res & 0xf) > 9);
    setC(res < max(max(val, val2), (unsigned char)(getC() ? 1 : 0)));
    val = res;
}

void CPU::sbc(unsigned char& val, unsigned char val2) {
    unsigned char res = val - val2 - (getC() ? 1 : 0);
    setZ(res == 0);
    setN(true);
    setH((res & 0xf) > 9);
    setC(val < (val2 + (getC() ? 1 : 0)));
    val = res;
}

void CPU::log_and(unsigned char val) {
    regs.a &= val;
    setZ(regs.a == 0);
    setN(false);
    setH(true);
    setC(false);
}

void CPU::log_or(unsigned char val) {
    regs.a |= val;
    setZ(regs.a == 0);
    setN(false);
    setH(false);
    setC(false);
}

void CPU::log_xor(unsigned char val) {
    regs.a ^= val;
    setZ(regs.a == 0);
    setN(false);
    setH(false);
    setC(false);
}

void CPU::cp(unsigned char val) {
    setZ(regs.a == val);
    setN(true);
    setH(((regs.a - val) & 0xf) > 9);
    setC(regs.a < val);
}

void CPU::sla(unsigned char& val) {
    setC((val & 0x80) != 0);
    val <<= 1;
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::push(unsigned short val) {
    sp -= 2;
    Emulator::write16(sp, val);
}

void CPU::pop(unsigned short& val) {
    val = Emulator::read16(sp);
    sp += 2;
}

void CPU::rlc(unsigned char& val) {
    unsigned char bit = (val & 0x80) ? 1 : 0;
    val <<= 1;
    val |= (getC() ? 1 : 0);
    setC(bit);
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::rrc(unsigned char& val) {
    unsigned char bit = val & 1;
    val >>= 1;
    val |= (getC() ? 1 : 0) << 7;
    setC(bit);
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::res(unsigned char& val, unsigned char bit) {
    val &= ~(1 << bit);
}

void CPU::set(unsigned char& val, unsigned char bit) {
    val |= 1 << bit;
}

void CPU::bit(unsigned char val, unsigned char bit) {
    setZ((val & (1 << bit)) == 0);
    setN(false);
    setH(true);
}

void CPU::srl(unsigned char& val) {
    setC(val & 1);
    val >>= 1;
    setZ(val == 0);
    setN(false);
    setH(false);
}

void print_op(unsigned char op, unsigned short data = 0) {
    #ifndef DEBUG
    return;
    #endif
    if (op == 0xcb) printf(cb_format[data]);
    else printf(op_format[op], data);
    printf("\n");
}

int CPU::exec() {
    unsigned char op = Emulator::read8(pc), 
    d8 = Emulator::read8(pc + 1);
    unsigned short d16 = Emulator::read16(pc + 1);
    unsigned short print_data = 0;
    bool jump = false;
    int jump_cycles = 0;
    switch (op) {
    case 0x00:
        break;
    case 0x01:
        print_data = d16;
        regs.bc = d16;
        break;
    case 0x03:
        regs.bc++;
        break;
    case 0x04:
        inc(regs.b);
        break;
    case 0x05:
        dec(regs.b);
        break;
    case 0x06:
        print_data = d8;
        regs.b = d8;
        break;
    case 0x07:
        rlc(regs.a);
        setZ(false);
        break;
    case 0x09: {
        unsigned short res = regs.hl + regs.bc;
        setN(false);
        setH((regs.hl & 0xf) > 9);
        setC(res < max(regs.hl, regs.bc));
        regs.hl = res;
        break;
    }
    case 0x0a:
        regs.a = Emulator::read8(regs.bc);
        break;
    case 0x0b:
        regs.bc--;
        break;
    case 0x0c:
        inc(regs.c);
        break;
    case 0x0d:
        dec(regs.c);
        break;
    case 0x0e:
        print_data = d8;
        regs.c = d8;
        break;
    case 0x11:
        print_data = d16;
        regs.de = d16;
        break;
    case 0x12:
        Emulator::write8(regs.de, regs.a);
        break;
    case 0x13:
        regs.de++;
        break;
    case 0x16:
        print_data = d8;
        regs.d = d8;
        break;
    case 0x18:
        print_data = pc + (signed char)d8 + op_len[op];
        pc += (signed char)d8;
        break;
    case 0x19: {
        unsigned short res = regs.hl + regs.de;
        setN(false);
        setH((regs.hl & 0xf) > 9);
        setC(res < max(regs.hl, regs.de));
        regs.hl = res;
        break;
    }
    case 0x1a:
        regs.a = Emulator::read8(regs.de);
        break;
    case 0x1b:
        regs.de--;
        break;
    case 0x1c:
        inc(regs.e);
        break;
    case 0x1d:
        dec(regs.e);
        break;
    case 0x1e:
        print_data = d8;
        regs.e = d8;
        break;
    case 0x20:
        print_data = pc + (signed char)d8 + op_len[op];
        if (!getZ()) {
            pc += (signed char)d8;
            jump_cycles = 4;
        }
        break;
    case 0x21:
        print_data = d16;
        regs.hl = d16;
        break;
    case 0x22:
        Emulator::write8(regs.hl++, regs.a);
        break;
    case 0x23:
        regs.hl++;
        break;
    case 0x25:
        dec(regs.h);
        break;
    case 0x26:
        print_data = d8;
        regs.h = d8;
        break;
    case 0x27:
        if (!getN()) {
            if (!getC() && !getH() && (regs.a & 0xf0 < 0xa0) && (regs.a && 0xf < 9)) {
                setC(false);
                break;
            }
            if (!getC()) {
                if (!getH()) {
                    if (regs.a & 0xf0 < 0xa0) {
                        if (regs.a & 0xf > 9) {
                            regs.a += 0x06;
                            setC(false);
                        }
                    }
                    else {
                        if (regs.a & 0xf > 9) {
                            regs.a += 0x66;
                        }
                        else {
                            regs.a += 0x60;
                        }
                        setC(true);
                    }
                }
                else {
                    if (regs.a & 0xf0 < 0xa0) {
                        regs.a += 0x06;
                        setC(false);
                    }
                    else {
                        regs.a += 0x66;
                        setC(true);
                    }
                }
            }
            else {
                if (!getH()) {
                    if (regs.a & 0xf > 9) {
                        regs.a += 0x66;
                    }
                    else {
                        regs.a += 0x60;
                    }
                    setC(true);
                }
                else {
                    regs.a += 0x66;
                    setC(true);
                }
            }
        }
        else {
            if (!getC() && !getH()) {
                setC(false);
            }
            else if (!getC()&& getH()) {
                regs.a += 0xfa;
                setC(false);
            }
            else {
                if (!getH()) {
                    regs.a += 0xa0;
                }
                else {
                    regs.a += 0x9a;
                }
                setC(true);
            }
        }
        break;
    case 0x28:
        print_data = pc + (signed char)d8 + op_len[op];
        if (getZ()) {
            pc += (signed char)d8;
            jump_cycles = 4;
        }
        break;
    case 0x2a:
        regs.a = Emulator::read8(regs.hl++);
        break;
    case 0x2b:
        regs.hl--;
        break;
    case 0x2c:
        inc(regs.l);
        break;
    case 0x2d:
        dec(regs.l);
        break;
    case 0x2e:
        print_data = d8;
        regs.l = d8;
        break;
    case 0x2f:
        regs.a ^= 0xff;
        setN(true);
        setH(true);
        break;
    case 0x30:
        print_data = pc + (signed char)d8 + op_len[op];
        if (!getC()) {
            pc += (signed char)d8;
            jump_cycles = 4;
        }
        break;
    case 0x31:
        print_data = d16;
        sp = d16;
        break;
    case 0x32:
        Emulator::write8(regs.hl--, regs.a);
        break;
    case 0x34:
        Emulator::write8(regs.hl, Emulator::read8(regs.hl) + 1);
        setZ(Emulator::read8(regs.hl) == 0);
        setN(false);
        setH((Emulator::read8(regs.hl) & 0xf) > 9);
        break;
    case 0x35:
        Emulator::write8(regs.hl, Emulator::read8(regs.hl) - 1);
        setZ(Emulator::read8(regs.hl) == 0);
        setN(true);
        setH((Emulator::read8(regs.hl) & 0xf) > 9);
        break;
    case 0x36:
        print_data = d8;
        Emulator::write8(regs.hl, d8);
        break;
    case 0x38:
        print_data = pc + (signed char)d8 + op_len[op];
        if (getC()) {
            pc += (signed char)d8;
            jump_cycles = 4;
        }
        break;
    case 0x3a:
        regs.a = Emulator::read8(regs.hl--);
        break;
    case 0x3c:
        inc(regs.a);
        break;
    case 0x3d:
        dec(regs.a);
        break;
    case 0x3e:
        print_data = d8;
        regs.a = d8;
        break;
    case 0x40:
        regs.b = regs.b;
        break;
    case 0x46:
        regs.b = Emulator::read8(regs.hl);
        break;
    case 0x47:
        regs.b = regs.a;
        break;
    case 0x4e:
        regs.c = Emulator::read8(regs.hl);
        break;
    case 0x4f:
        regs.c = regs.a;
        break;
    case 0x54:
        regs.d = regs.h;
        break;
    case 0x56:
        regs.d = Emulator::read8(regs.hl);
        break;
    case 0x57:
        regs.d = regs.a;
        break;
    case 0x5d:
        regs.e = regs.l;
        break;
    case 0x5e:
        regs.e = Emulator::read8(regs.hl);
        break;
    case 0x5f:
        regs.e = regs.a;
        break;
    case 0x60:
        regs.h = regs.b;
        break;
    case 0x61:
        regs.h = regs.c;
        break;
    case 0x62:
        regs.h = regs.d;
        break;
    case 0x67:
        regs.h = regs.a;
        break;
    case 0x69:
        regs.l = regs.c;
        break;
    case 0x6b:
        regs.l = regs.e;
        break;
    case 0x6f:
        regs.l = regs.a;
        break;
    case 0x70:
        Emulator::write8(regs.hl, regs.b);
        break;
    case 0x71:
        Emulator::write8(regs.hl, regs.c);
        break;
    case 0x72:
        Emulator::write8(regs.hl, regs.d);
        break;
    case 0x73:
        Emulator::write8(regs.hl, regs.e);
        break;
    case 0x77:
        Emulator::write8(regs.hl, regs.a);
        break;
    case 0x78:
        regs.a = regs.b;
        break;
    case 0x79:
        regs.a = regs.c;
        break;
    case 0x7a:
        regs.a = regs.d;
        break;
    case 0x7b:
        regs.a = regs.e;
        break;
    case 0x7c:
        regs.a = regs.h;
        break;
    case 0x7d:
        regs.a = regs.l;
        break;
    case 0x7e:
        regs.a = Emulator::read8(regs.hl);
        break;
    case 0x80:
        add(regs.a, regs.b);
        break;
    case 0x82:
        add(regs.a, regs.d);
        break;
    case 0x85:
        add(regs.a, regs.l);
        break;
    case 0x86:
        add(regs.a, Emulator::read8(regs.hl));
        break;
    case 0x87:
        add(regs.a, regs.a);
        break;
    case 0x89:
        adc(regs.a, regs.c);
        break;
    case 0x8e:
        adc(regs.a, Emulator::read8(regs.hl));
        break;
    case 0x90:
        sub(regs.a, regs.b);
        break;
    case 0x96:
        sub(regs.a, Emulator::read8(regs.hl));
        break;
    case 0x99:
        sbc(regs.a, regs.c);
        break;
    case 0xa0:
        log_and(regs.b);
        break;
    case 0xa1:
        log_and(regs.c);
        break;
    case 0xa7:
        log_and(regs.a);
        break;
    case 0xa8:
        log_xor(regs.b);
        break;
    case 0xa9:
        log_xor(regs.c);
        break;
    case 0xaf:
        log_xor(regs.a);
        break;
    case 0xb0:
        log_or(regs.b);
        break;
    case 0xb1:
        log_or(regs.c);
        break;
    case 0xb2:
        log_or(regs.d);
        break;
    case 0xb7:
        log_or(regs.a);
        break;
    case 0xb8:
        cp(regs.b);
        break;
    case 0xb9:
        cp(regs.c);
        break;
    case 0xbe:
        cp(Emulator::read8(regs.hl));
        break;
    case 0xc0:
        if (!getZ()) {
            pop(pc);
            jump = true;
            jump_cycles = 12;
        }
        break;
    case 0xc1:
        pop(regs.bc);
        break;
    case 0xc2:
        print_data = d16;
        if (!getZ()) {
            pc = d16;
            jump = true;
            jump_cycles = 4;
        }
        break;
    case 0xc3:
        print_data = d16;
        pc = d16;
        jump = true;
        break;
    case 0xc5:
        push(regs.bc);
        break;
    case 0xc6:
        print_data = d8;
        add(regs.a, d8);
        break;
    case 0xc8:
        if (getZ()) {
            pop(pc);
            jump = true;
            jump_cycles = 12;
        }
        break;
    case 0xc9:
        pop(pc);
        jump = true;
        break;
    case 0xca:
        print_data = d16;
        if (getZ()) {
            pc = d16;
            jump = true;
            jump_cycles = 4;
        }
        break;
    case 0xcb:
        return exec_cb();
    case 0xcd:
        print_data = d16;
        push(pc + op_len[op]);
        pc = d16;
        jump = true;
        break;
    case 0xd0:
        if (!getC()) {
            pop(pc);
            jump = true;
            jump_cycles = 12;
        }
        break;
    case 0xd1:
        pop(regs.de);
        break;
    case 0xd5:
        push(regs.de);
        break;
    case 0xd6:
        print_data = d8;
        sub(regs.a, d8);
        break;
    case 0xd9:
        Emulator::enable_ints();
        pop(pc);
        jump = true;
        break;
    case 0xde:
        print_data = d8;
        sbc(regs.a, d8);
        break;
    case 0xe0:
        print_data = d8;
        Emulator::write8(0xff00 + d8, regs.a);
        break;
    case 0xe1:
        pop(regs.hl);
        break;
    case 0xe2:
        Emulator::write8(0xff00 + regs.c, regs.a);
        break;
    case 0xe5:
        push(regs.hl);
        break;
    case 0xe6:
        print_data = d8;
        log_and(d8);
        break;
    case 0xe9:
        pc = regs.hl;
        jump = true;
        break;
    case 0xea:
        print_data = d16;
        Emulator::write8(d16, regs.a);
        break;
    case 0xee:
        print_data = d8;
        log_xor(d8);
        break;
    case 0xef:
        push(pc + op_len[op]);
        pc = 0x28;
        jump = true;
        break;
    case 0xf0:
        print_data = d8;
        regs.a = Emulator::read8(0xff00 + d8);
        break;
    case 0xf1:
        pop(regs.af);
        break;
    case 0xf3:
        Emulator::disable_ints();
        break;
    case 0xf5:
        push(regs.af);
        break;
    case 0xf6:
        print_data = d8;
        log_or(d8);
        break;
    case 0xfa:
        print_data = d16;
        regs.a = Emulator::read8(d16);
        break;
    case 0xfb:
        Emulator::enable_ints();
        break;
    case 0xfe:
        print_data = d8;
        cp(d8);
        break;
    default:
        std::cout << "Unimplemented opcode: " << std::hex << (unsigned)op << " at " << pc << '\n';
        return 0;
    }
    if (!jump) pc += op_len[op];
    print_op(op, print_data);
    return op_cycles[op] + jump_cycles;
}

int CPU::exec_cb() {
    unsigned char op = Emulator::read8(pc + 1);
    switch (op) {
    case 0x27:
        sla(regs.a);
        break;
    case 0x33:
        swap(regs.e);
        break;
    case 0x37:
        swap(regs.a);
        break;
    case 0x3f:
        srl(regs.a);
        break;
    case 0x40:
        bit(regs.b, 0);
        break;
    case 0x41:
        bit(regs.c, 0);
        break;
    case 0x47:
        bit(regs.a, 0);
        break;
    case 0x48:
        bit(regs.b, 1);
        break;
    case 0x50:
        bit(regs.b, 2);
        break;
    case 0x57:
        bit(regs.a, 2);
        break;
    case 0x58:
        bit(regs.b, 3);
        break;
    case 0x5f:
        bit(regs.a, 3);
        break;
    case 0x60:
        bit(regs.b, 4);
        break;
    case 0x61:
        bit(regs.c, 4);
        break;
    case 0x68:
        bit(regs.b, 5);
        break;
    case 0x69:
        bit(regs.c, 5);
        break;
    case 0x6f:
        bit(regs.a, 5);
        break;
    case 0x70:
        bit(regs.b, 6);
        break;
    case 0x71:
        bit(regs.c, 6);
        break;
    case 0x77:
        bit(regs.a, 6);
        break;
    case 0x78:
        bit(regs.b, 7);
        break;
    case 0x79:
        bit(regs.c, 7);
        break;
    case 0x7e:
        bit(Emulator::read8(regs.hl), 7);
        break;
    case 0x7f:
        bit(regs.a, 7);
        break;
    case 0x86: {
        auto val = Emulator::read8(regs.hl);
        res(val, 0);
        Emulator::write8(regs.hl, val);
        break;
    }
    case 0x87:
        res(regs.a, 0);
        break;
    case 0x9e: {
        auto val = Emulator::read8(regs.hl);
        res(val, 3);
        Emulator::write8(regs.hl, val);
        break;
    }
    case 0xbe: {
        auto val = Emulator::read8(regs.hl);
        res(val, 7);
        Emulator::write8(regs.hl, val);
        break;
    }
    case 0xde: {
        auto val = Emulator::read8(regs.hl);
        set(val, 3);
        Emulator::write8(regs.hl, val);
        break;
    }
    case 0xfe: {
        auto val = Emulator::read8(regs.hl);
        set(val, 7);
        Emulator::write8(regs.hl, val);
        break;
    }
    default:
        std::cout << "Unimplemented CB opcode: " << std::hex << (unsigned)op << " at " << pc << '\n';
        return 0;
    }
    print_op(0xcb, op);
    pc += 2;
    return cb_cycles[op & 0xf];
}
