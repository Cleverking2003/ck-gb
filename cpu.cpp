#include "cpu.hpp"
#include "emul.hpp"
#include "ops.hpp"
#include <iostream>

void CPU::inc(unsigned char& val) {
    setZ(++val == 0);
    setN(false);
    setH((val & 0xf) == 0);
}

void CPU::dec(unsigned char& val) {
    setZ(--val == 0);
    setN(true);
    setH((val & 0xf) == 0xf);
}

void CPU::swap(unsigned char& val) {
    unsigned char lo = val & 0xf, hi = (val >> 4) & 0xf;
    val = (lo << 4) | hi;
    setZ(val == 0);
    setN(false);
    setH(false);
    setC(false);
}

void CPU::add(unsigned char val) {
    unsigned char res = regs.a + val;
    setZ(res == 0);
    setN(false);
    setH(((regs.a & 0xf) + (val & 0xf)) > 0xf);
    setC(regs.a > (0xff - val));
    regs.a = res;
}

void CPU::add16(unsigned short& val, unsigned short val2) {
    unsigned short res = val + val2;
    setN(false);
    setH(((val & 0xfff) + (val2 & 0xfff)) > 0xfff);
    setC(val > (0xffff - val2));
    val = res;
}

void CPU::sub(unsigned char val) {
    auto res = regs.a - val;
    setZ(res == 0);
    setN(true);
    setH((regs.a & 0xf) < (val & 0xf));
    setC(regs.a < val);
    regs.a = res;
}

void CPU::adc(unsigned char val) {
    unsigned char c = getC() ? 1 : 0;
    unsigned char res = regs.a + val + c;
    setZ(res == 0);
    setN(false);
    setH(((regs.a & 0xf) + (val & 0xf) + c) > 0xf);
    setC(((int)regs.a + val + c) > 0xff);
    regs.a = res;
}

void CPU::sbc(unsigned char val) {
    unsigned char c = getC() ? 1 : 0;
    unsigned char res = regs.a - val - c;
    setZ(res == 0);
    setN(true);
    setH((regs.a & 0xf) < ((val & 0xf) + c));
    setC((regs.a < (val + c)) || (regs.a < val));
    regs.a = res;
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
    setH((regs.a & 0xf) < (val & 0xf));
    setC(regs.a < val);
}

void CPU::sla(unsigned char& val) {
    setC((val & 0x80) != 0);
    val <<= 1;
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::sra(unsigned char& val) {
    auto bit = val & 0x80;
    setC((val & 1) != 0);
    val >>= 1;
    val |= bit;
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
    val |= bit;
    setC(bit);
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::rrc(unsigned char& val) {
    unsigned char bit = val & 1;
    val >>= 1;
    val |= bit << 7;
    setC(bit);
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::rl(unsigned char& val) {
    unsigned char bit = (val & 0x80) ? 1 : 0;
    val <<= 1;
    val |= (getC() ? 1 : 0);
    setC(bit);
    setZ(val == 0);
    setN(false);
    setH(false);
}

void CPU::rr(unsigned char& val) {
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
    unsigned char regs8[] = { regs.b, regs.c, regs.d, regs.e, regs.h, regs.l, Emulator::read8(regs.hl), regs.a };
    unsigned char* reg8_refs[] = { &regs.b, &regs.c, &regs.d, &regs.e, &regs.h, &regs.l, nullptr, &regs.a };
    unsigned short regs16[] = { regs.bc, regs.de, regs.hl, sp };
    unsigned short* reg16_refs[] = { &regs.bc, &regs.de, &regs.hl, &sp };
    unsigned short regs16_stack[] = { regs.bc, regs.de, regs.hl, regs.af };
    unsigned short* reg16_stack_refs[] = { &regs.bc, &regs.de, &regs.hl, &regs.af };
    bool jump_flags[] = { !getZ(), getZ(), true, !getC(), getC(), true };
    switch (op) {
    case 0x00:
        break;
    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
        print_data = d16;
        *reg16_refs[(op & 0x30) >> 4] = d16;
        break;
    case 0x02:
        Emulator::write8(regs.bc, regs.a);
        break;
    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
        (*reg16_refs[(op & 0x30) >> 4])++;
        break;
    case 0x04: case 0x0c:
    case 0x14: case 0x1c:
    case 0x24: case 0x2c:
    case 0x34: case 0x3c: {
        auto reg = (op & 0x38) >> 3;
        if (reg == 6) {
            unsigned char val = Emulator::read8(regs.hl);
            val++;
            Emulator::write8(regs.hl, val);
            setZ(val == 0);
            setN(false);
            setH((val & 0xf) == 0);
        }
        else inc(*reg8_refs[reg]);
        break;
    }
    case 0x05: case 0x0d:
    case 0x15: case 0x1d:
    case 0x25: case 0x2d:
    case 0x35: case 0x3d: {
        auto reg = (op & 0x38) >> 3;
        if (reg == 6) {
            unsigned char val = Emulator::read8(regs.hl);
            val--;
            Emulator::write8(regs.hl, val);
            setZ(val == 0);
            setN(true);
            setH((val & 0xf) == 0xf);
        }
        else dec(*reg8_refs[reg]);
        break;
    }
    case 0x06: case 0x0e:
    case 0x16: case 0x1e:
    case 0x26: case 0x2e:
    case 0x36: case 0x3e: {
        print_data = d8;
        auto reg = (op & 0x38) >> 3;
        if (reg == 6) {
            Emulator::write8(regs.hl, d8);
        }
        else *reg8_refs[reg] = d8;
        break;
    }
    case 0x07:
        rlc(regs.a);
        setZ(false);
        break;
    case 0x08:
        print_data = d16;
        Emulator::write16(d16, sp);
        break;
    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:
        add16(regs.hl, regs16[(op & 0x30) >> 4]);
        break;
    case 0x0a:
        regs.a = Emulator::read8(regs.bc);
        break;
    case 0x0b:
    case 0x1b:
    case 0x2b:
    case 0x3b:
        (*reg16_refs[(op & 0x30) >> 4])--;
        break;
    case 0x0f:
        rrc(regs.a);
        setZ(false);
        break;
    case 0x10:
        m_running = false;
        break;
    case 0x12:
        Emulator::write8(regs.de, regs.a);
        break;
    case 0x17:
        rl(regs.a);
        setZ(false);
        break;
    case 0x18:
        print_data = pc + (signed char)d8 + op_len[op];
        pc += (signed char)d8;
        break;
    case 0x1a:
        regs.a = Emulator::read8(regs.de);
        break;
    case 0x1f:
        rr(regs.a);
        setZ(false);
        break;
    case 0x20:
        print_data = pc + (signed char)d8 + op_len[op];
        if (!getZ()) {
            pc += (signed char)d8;
            jump_cycles = 4;
        }
        break;
    case 0x22:
        Emulator::write8(regs.hl++, regs.a);
        break;
    case 0x27: {
        bool n = getN(), h = getH(), c = getC(), new_c = false;
        unsigned char addend = 0;
        if ((((regs.a & 0xf) > 9) && !n) || h) {
            addend |= 0x06;
        }
        if (((regs.a > 0x99) && !n) || c) {
            new_c = true;
            addend |= 0x60;

        }
        regs.a += n ? -addend : addend;
        setZ(regs.a == 0);
        setH(false);
        setC(new_c);
        break;
    }
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
    case 0x32:
        Emulator::write8(regs.hl--, regs.a);
        break;
    case 0x37:
        setN(false);
        setH(false);
        setC(true);
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
    case 0x3f:
        setN(false);
        setH(false);
        setC((getC() ? 1 : 0) ^ 1);
        break;
    case 0x40 ... 0x7f: {
        if (op == 0x76) {
            m_running = false;
            break;
        }
        auto reg1 = op & 7, reg2 = (op & 0x38) >> 3;
        auto val = regs8[reg1];
        if (reg2 == 6) {
            Emulator::write8(regs.hl, val);
        }
        else {
            *reg8_refs[reg2] = val;
        }
        break;
    }
    case 0x80 ... 0xbf:
    case 0xc6: case 0xce:
    case 0xd6: case 0xde:
    case 0xe6: case 0xee:
    case 0xf6: case 0xfe: {
        auto action = op & 0x38, reg = op & 0x7;
        auto val = regs8[reg];
        if ((op & 0xc0) == 0xc0) print_data = val = d8;
        switch (action) {
            case 0x00: add(val); break;
            case 0x08: adc(val); break;
            case 0x10: sub(val); break;
            case 0x18: sbc(val); break;
            case 0x20: log_and(val); break;
            case 0x28: log_xor(val); break;
            case 0x30: log_or(val); break;
            case 0x38: cp(val); break;
        }
        break;
    }
    case 0xc0: case 0xd0:
    case 0xc8: case 0xd8:
    case 0xc9: case 0xd9: {
        auto flag_num = ((op & 0x10) ? 3 : 0 ) + ((op & 0x8) ? 1 : 0) + (op & 1);
        if (jump_flags[flag_num]) {
            pop(pc);
            jump = true;
            if (!(op & 1)) jump_cycles = 12;
        }
        if (op == 0xd9) Emulator::enable_ints();
        break;
    }
    case 0xc1: case 0xd1:
    case 0xe1: case 0xf1:
        pop(*reg16_stack_refs[(op & 0x30) >> 4]);
        regs.af &= ~0xf;
        break;
    case 0xc2: case 0xd2:
    case 0xca: case 0xda: {
        print_data = d16;
        auto flag_num = ((op & 0x10) ? 3 : 0) + ((op & 0x8) ? 1 : 0);
        if (jump_flags[flag_num]) {
            pc = d16;
            jump = true;
            jump_cycles = 4;
        }
        break;
    }
    case 0xc3:
        print_data = d16;
        pc = d16;
        jump = true;
        break;
    case 0xc4: case 0xcc: case 0xcd:
    case 0xd4: case 0xdc: {
        print_data = d16;
        auto flag_num = ((op & 0x10) ? 3 : 0) + ((op & 0x8) ? 1 : 0) + (op & 0x1);
        if (jump_flags[flag_num]) {
            push(pc + op_len[op]);
            pc = d16;
            jump = true;
            jump_cycles = 12;
        }
        break;
    }
    case 0xc5: case 0xd5:
    case 0xe5: case 0xf5:
        push(regs16_stack[(op & 0x30) >> 4]);
        break;
    case 0xc7: case 0xcf:
    case 0xd7: case 0xdf:
    case 0xe7: case 0xef:
    case 0xf7: case 0xff:
        push(pc + op_len[op]);
        pc = op & 0x38;
        jump = true;
        break;
    case 0xcb:
        return exec_cb();
    case 0xe0:
        print_data = d8;
        Emulator::write8(0xff00 + d8, regs.a);
        break;
    case 0xe2:
        Emulator::write8(0xff00 + regs.c, regs.a);
        break;
    case 0xe8: {
        print_data = d8;
        signed char r8 = d8;
        setZ(false);
        setN(false);
        if (r8 > 0) {
            setH(((sp & 0xf) + (r8 & 0xf)) > 0xf);
            setC(((sp & 0xff) + r8) > 0xff);
        }
        else {
            d8 = -r8;
            setH((sp & 0xf) < (d8 & 0xf));
            setC((sp & 0xff) < d8);
        }
        sp += r8;
        break;
    }
    case 0xe9:
        pc = regs.hl;
        jump = true;
        break;
    case 0xea:
        print_data = d16;
        Emulator::write8(d16, regs.a);
        break;
    case 0xf0:
        print_data = d8;
        regs.a = Emulator::read8(0xff00 + d8);
        break;
    case 0xf2:
        regs.a = Emulator::read8(0xff00 + regs.c);
        break;
    case 0xf3:
        Emulator::disable_ints();
        break;
    case 0xf8: {
        print_data = d8;
        signed char r8 = d8;
        setZ(false);
        setN(false);
        if (r8 > 0) {
            setH(((sp & 0xf) + (r8 & 0xf)) > 0xf);
            setC(((sp & 0xff) + r8) > 0xff);
        }
        else {
            d8 = -r8;
            setH((sp & 0xf) < (d8 & 0xf));
            setC((sp & 0xff) < d8);
        }
        regs.hl = sp + r8;
        break;
    }
    case 0xf9:
        sp = regs.hl;
        break;
    case 0xfa:
        print_data = d16;
        regs.a = Emulator::read8(d16);
        break;
    case 0xfb:
        Emulator::enable_ints();
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
    auto op = Emulator::read8(pc + 1);
    auto action = op & 0xf8, reg = op & 0x7, bit_num = (op & 0x38) >> 3;
    unsigned char regs8[] = { regs.b, regs.c, regs.d, regs.e, regs.h, regs.l, Emulator::read8(regs.hl), regs.a };
    unsigned char* reg_refs[] = { &regs.b, &regs.c, &regs.d, &regs.e, &regs.h, &regs.l, nullptr, &regs.a };
    auto val = regs8[reg];
    switch (action) {
    case 0x00:
        rlc(val);
        break;
    case 0x08:
        rrc(val);
        break;
    case 0x10:
        rl(val);
        break;
    case 0x18:
        rr(val);
        break;
    case 0x20:
        sla(val);
        break;
    case 0x28:
        sra(val);
        break;
    case 0x30:
        swap(val);
        break;
    case 0x38:
        srl(val);
        break;
    case 0x40 ... 0x78:
        bit(val, bit_num);
        break;
    case 0x80 ... 0xb8:
        res(val, bit_num);
        break;
    case 0xc0 ... 0xf8:
        set(val, bit_num);
        break;
    }
    switch (reg) {
    case 0x06:
        Emulator::write8(regs.hl, val);
        break;
    default:
        *reg_refs[reg] = val;
        break;
    }
    print_op(0xcb, op);
    pc += 2;
    return cb_cycles[op & 0xf];
}
