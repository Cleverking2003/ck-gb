#include "cpu.hpp"
#include "emul.hpp"
#include <iostream>

void CPU::inc(unsigned char& val) {
    regs.flags.z = (++val == 0);
    regs.flags.n = 0;
    regs.flags.h = (val & 0xf) > 9;
}

void CPU::dec(unsigned char& val) {
    regs.flags.z = (--val == 0);
    regs.flags.n = 1;
    regs.flags.h = (val & 0xf) > 9;
}

void CPU::swap(unsigned char& val) {
    unsigned char lo = val & 0xf, hi = (val >> 4) & 0xf;
    val = (lo << 4) | hi;
    regs.flags.z = (val == 0);
    regs.flags.n = regs.flags.h = regs.flags.c = 0;
}

void CPU::add(unsigned char& val, unsigned char val2) {
    regs.flags.c = ((val + val2) < max(val, val2));
    val += val2;
    regs.flags.z = (val == 0);
    regs.flags.n = 0;
    regs.flags.h = (val & 0xf) > 9;
}

void CPU::sub(unsigned char& val, unsigned char val2) {
    regs.flags.c = (val < val2);
    val -= val2;
    regs.flags.z = (val == 0);
    regs.flags.n = 1;
    regs.flags.h = (val & 0xf) > 9;
}

void CPU::adc(unsigned char& val, unsigned char val2) {
    unsigned char res = val + val2 + regs.flags.c;
    regs.flags.z = (res == 0);
    regs.flags.n = 0;
    regs.flags.h = (res & 0xf) > 9;
    regs.flags.c = (res < max(max(val, val2), (unsigned char)regs.flags.c));
    val = res;
}

void CPU::sbc(unsigned char& val, unsigned char val2) {
    unsigned char res = val - val2 - regs.flags.c;
    regs.flags.z = (res == 0);
    regs.flags.n = 1;
    regs.flags.h = (res & 0xf) > 9;
    regs.flags.c = (val < (val2 + regs.flags.c));
    val = res;
}

void CPU::log_and(unsigned char val) {
    regs.a &= val;
    regs.flags.z = (regs.a == 0);
    regs.flags.n = regs.flags.c = 0;
    regs.flags.h = 1;
}

void CPU::log_or(unsigned char val) {
    regs.a |= val;
    regs.flags.z = (regs.a == 0);
    regs.flags.n = regs.flags.h = regs.flags.c = 0;
}

void CPU::log_xor(unsigned char val) {
    regs.a ^= val;
    regs.flags.z = (regs.a == 0);
    regs.flags.n = regs.flags.h = regs.flags.c = 0;
}

void CPU::cp(unsigned char val) {
    regs.flags.z = (regs.a == val);
    regs.flags.n = 1;
    regs.flags.h = ((regs.a - val) & 0xf) > 9;
    regs.flags.c = (regs.a < val);
}

void CPU::push(unsigned short val) {
    sp -= 2;
    m_emul->write16(sp, val);
}

void CPU::pop(unsigned short& val) {
    val = m_emul->read16(sp);
    sp += 2;
}

void CPU::rlc(unsigned char& val) {
    bool bit = val & 0x80;
    val <<= 1;
    val |= regs.flags.c;
    regs.flags.c = bit;
    regs.flags.z = (val == 0);
    regs.flags.n = regs.flags.h = 0;
}

void CPU::rrc(unsigned char& val) {
    bool bit = val & 1;
    val >>= 1;
    val |= regs.flags.c << 7;
    regs.flags.c = bit;
    regs.flags.z = (val == 0);
    regs.flags.n = regs.flags.h = 0;
}

void CPU::res(unsigned char& val, unsigned char bit) {
    val &= ~(1 << bit);
}

void CPU::set(unsigned char& val, unsigned char bit) {
    val |= 1 << bit;
}

bool CPU::exec() {
    unsigned char op = m_emul->read8(pc++);
    switch (op) {
    case 0x00:
        std::cout << "nop" << '\n';
        break;
    case 0x01:
        std::cout << "ld bc, " << std::hex << (unsigned)m_emul->read16(pc) << '\n';
        regs.bc = m_emul->read16(pc);
        pc += 2;
        break;
    case 0x03:
        std::cout << "inc bc\n";
        regs.bc++;
        break;
    case 0x04:
        std::cout << "inc b\n";
        inc(regs.b);
        break;
    case 0x05:
        dec(regs.b);
        std::cout << "dec b" << '\n';
        break;
    case 0x06:
        regs.b = m_emul->read8(pc++);
        std::cout << "ld b, " << std::hex << (unsigned)regs.b << '\n';
        break;
    case 0x07:
        std::cout << "rlca\n";
        rlc(regs.a);
        regs.flags.z = 0;
        break;
    case 0x0b:
        regs.bc--;
        std::cout << "dec bc" << '\n';
        break;
    case 0x0c:
        inc(regs.c);
        std::cout << "inc c" << '\n';
        break;
    case 0x0d:
        dec(regs.c);
        std::cout << "dec c" << '\n';
        break;
    case 0x0e:
        regs.c = m_emul->read8(pc++);
        std::cout << "ld c, " << std::hex << (unsigned)regs.c << '\n';
        break;
    case 0x11:
        std::cout << "ld de, " << m_emul->read16(pc) << '\n';
        regs.de = m_emul->read16(pc);
        pc += 2;
        break;
    case 0x12:
        std::cout << "ld (de), a\n";
        m_emul->write8(regs.de, regs.a);
        break;
    case 0x13:
        std::cout << "inc de\n";
        regs.de++;
        break;
    case 0x16:
        regs.d = m_emul->read8(pc++);
        std::cout << "ld d, " << std::hex << (unsigned)regs.d << '\n';
        break;
    case 0x18:
        std::cout << "jr " << m_emul->read8(pc) << '\n';
        pc += (signed char)m_emul->read8(pc);
        break;
    case 0x19: {
        std::cout << "add hl, de\n";
        unsigned short res = regs.hl + regs.de;
        regs.flags.n = 0;
        regs.flags.h = (regs.hl & 0xf) > 9;
        regs.flags.c = res < max(regs.hl, regs.de);
        regs.hl = res;
        break;
    }
    case 0x1a:
        std::cout << "ld a, (de)\n";
        regs.a = m_emul->read8(regs.de);
        break;
    case 0x1c:
        std::cout << "inc e\n";
        inc(regs.e);
        break;
    case 0x20:
        std::cout << "jr nz " << std::hex << (signed)m_emul->read8(pc) << '\n';
        if (!regs.flags.z) pc += (signed char)m_emul->read8(pc++);
        else pc++;
        break;
    case 0x21:
        regs.hl = m_emul->read16(pc);
        pc += 2;
        std::cout << "ld hl, " << std::hex << (unsigned)regs.hl << '\n';
        break;
    case 0x22:
        std::cout << "ld (hl+), a\n";
        m_emul->write8(regs.hl++, regs.a);
        break;
    case 0x23:
        std::cout << "inc hl\n";
        regs.hl++;
        break;
    case 0x28:
        std::cout << "jr z, " << (unsigned)m_emul->read8(pc) << '\n';
        if (regs.flags.z) pc += (signed char)m_emul->read8(pc++);
        else pc++;
        break;
    case 0x2a:
        std::cout << "ld a, (hl+)\n";
        regs.a = m_emul->read8(regs.hl++);
        break;
    case 0x2f:
        std::cout << "cpl\n";
        regs.a ^= 0xff;
        regs.flags.n = regs.flags.h = 1;
        break;
    case 0x31:
        std::cout << "ld sp, " << std::hex << (unsigned)m_emul->read16(pc);
        sp = m_emul->read16(pc);
        pc += 2;
        break;
    case 0x32:
        m_emul->write8(regs.hl--, regs.a);
        std::cout << "ld (hl-), a\n";
        break;
    case 0x36:
        std::cout << "ld (hl), " << std::hex << (unsigned)m_emul->read8(pc) << '\n';
        m_emul->write8(regs.hl, m_emul->read8(pc++));
        break;
    case 0x3e:
        regs.a = m_emul->read8(pc++);
        std::cout << "ld a, " << std::hex << (unsigned)regs.a << '\n';
        break;
    case 0x47:
        std::cout << "ld b, a\n";
        regs.b = regs.a;
        break;
    case 0x4f:
        std::cout << "ld c, a\n";
        regs.c = regs.a;
        break;
    case 0x56:
        std::cout << "ld d, (hl)\n";
        regs.d = m_emul->read8(regs.hl);
        break;
    case 0x5e:
        std::cout << "ld e, (hl)\n";
        regs.e = m_emul->read8(regs.hl);
        break;
    case 0x5f:
        std::cout << "ld e, a\n";
        regs.e = regs.a;
        break;
    case 0x78:
        std::cout << "ld a, b\n";
        regs.a = regs.b;
        break;
    case 0x79:
        std::cout << "ld a, c\n";
        regs.a = regs.c;
        break;
    case 0x7c:
        std::cout << "ld a, h\n";
        regs.a = regs.h;
        break;
    case 0x7e:
        std::cout << "ld a, (hl)\n";
        regs.a = m_emul->read8(regs.hl);
        break;
    case 0x87:
        std::cout << "add a,a\n";
        add(regs.a, regs.a);
        break;
    case 0xa1:
        std::cout << "and c\n";
        log_and(regs.c);
        break;
    case 0xa7:
        std::cout << "and a\n";
        log_and(regs.a);
        break;
    case 0xa9:
        std::cout << "xor c\n";
        log_xor(regs.c);
        break;
    case 0xaf:
        std::cout << "xor a, a" << '\n';
        log_xor(regs.a);
        break;
    case 0xb0:
        std::cout << "or b\n";
        log_or(regs.b);
        break;
    case 0xb1:
        std::cout << "or c\n";
        log_or(regs.c);
        break;
    case 0xc1:
        std::cout << "pop bc\n";
        pop(regs.bc);
        break;
    case 0xc3:
        pc = m_emul->read16(pc);
        std::cout << "jp " << std::hex << pc << '\n';
        break;
    case 0xc5:
        std::cout << "push bc\n";
        push(regs.bc);
        break;
    case 0xc8:
        std::cout << "ret z\n";
        if (regs.flags.z) pop(pc);
        break;
    case 0xc9:
        std::cout << "ret\n";
        pop(pc);
        break;
    case 0xca:
        std::cout << "jp z, " << m_emul->read16(pc) << '\n';
        if (regs.flags.z) pc = m_emul->read16(pc);
        else pc += 2;
        break;
    case 0xcb:
        return exec_cb();
    case 0xcd:
        std::cout << "call " << std::hex << (unsigned)m_emul->read16(pc) << '\n';
        push(pc + 2);
        pc = m_emul->read16(pc);
        break;
    case 0xd1:
        std::cout << "pop de\n";
        pop(regs.de);
        break;
    case 0xd5:
        std::cout << "push de\n";
        push(regs.de);
        break;
    case 0xe0:
        std::cout << "ldh (" << std::hex << (unsigned)m_emul->read8(pc) << "), a\n";
        m_emul->write8(0xff00 + m_emul->read8(pc++), regs.a);
        break;
    case 0xe1:
        std::cout << "pop hl\n";
        pop(regs.hl);
        break;
    case 0xe2:
        std::cout << "ld (c), a\n";
        m_emul->write8(0xff00 + regs.c, regs.a);
        break;
    case 0xe5:
        std::cout << "push hl\n";
        push(regs.hl);
        break;
    case 0xe6:
        std::cout << "and " << std::hex << (unsigned)m_emul->read8(pc) << "\n";
        log_and(m_emul->read8(pc++));
        break;
    case 0xe9:
        std::cout << "jp hl\n";
        pc = regs.hl;
        break;
    case 0xea:
        std::cout << "ld (" << std::hex << (unsigned)m_emul->read16(pc) << "), a\n";
        m_emul->write8(m_emul->read16(pc), regs.a);
        pc += 2;
        break;
    case 0xef:
        std::cout << "rst 28\n";
        push(pc);
        pc = 0x28;
        break;
    case 0xf0:
        std::cout << "ldh a, (" << std::hex << (unsigned)m_emul->read8(pc) << ")\n";
        regs.a = m_emul->read8(0xff00 + m_emul->read8(pc++));
        break;
    case 0xf1:
        std::cout << "pop af\n";
        pop(regs.af);
        break;
    case 0xf3:
        m_interrupts_enabled = false;
        std::cout << "di" << '\n';
        break;
    case 0xf5:
        std::cout << "push af\n";
        push(regs.af);
        break;
    case 0xfa:
        std::cout << "ld a, (" << m_emul->read16(pc) << ")\n";
        regs.a = m_emul->read8(m_emul->read16(pc));
        pc += 2;
        break;
    case 0xfb:
        m_interrupts_enabled = true;
        std::cout << "ei" << '\n';
        break;
    case 0xfe:
        std::cout << "cp " << (unsigned)m_emul->read8(pc) << '\n';
        cp(m_emul->read8(pc++));
        break;
    default:
        std::cout << "Unimplemented opcode: " << std::hex << (unsigned)op << '\n';
        return false;
    }
    return true;
}

bool CPU::exec_cb() {
    unsigned char op = m_emul->read8(pc++);
    switch (op) {
    case 0x37:
        std::cout << "swap a\n";
        swap(regs.a);
        break;
    case 0x87:
        std::cout << "res 0, a\n";
        res(regs.a, 0);
        break;
    default:
        std::cout << "Unimplemented CB opcode: " << std::hex << (unsigned)op << '\n';
        return false;
    }
    return true;
}
