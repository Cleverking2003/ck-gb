#include "cpu.hpp"
#include "emul.hpp"
#include <iostream>

void CPU::inc8(unsigned char& val) {
    regs.flags.z = regs.flags.c = (val++ == 0xff);
}

void CPU::inc16(unsigned short& val) {
    regs.flags.z = regs.flags.c = (val++ == 0xffff);
}

void CPU::dec8(unsigned char& val) {
    regs.flags.c = (val == 0);
    regs.flags.z = (--val == 0);
}

void CPU::dec16(unsigned short& val) {
    regs.flags.c = (val == 0);
    regs.flags.z = (--val == 0);
}

void CPU::swap(unsigned char& val) {
    unsigned char lo = val & 0xf, hi = (val >> 4) & 0xf;
    val = (lo << 4) | hi;
    regs.flags.z = (val == 0);
}

void CPU::add8(unsigned char& val, unsigned char& val2) {
    regs.flags.c = ((int)val + val2) > 0xff;
    val += val2;
    regs.flags.z = (val == 0);
}

void CPU::add16(unsigned short& val, unsigned short& val2) {
    regs.flags.c = ((int)val + val2) > 0xffff;
    val += val2;
    regs.flags.z = (val == 0);
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
    case 0x05:
        dec8(regs.b);
        std::cout << "dec b" << '\n';
        break;
    case 0x06:
        regs.b = m_emul->read8(pc++);
        std::cout << "ld b, " << std::hex << (unsigned)regs.b << '\n';
        break;
    case 0x0b:
        dec16(regs.bc);
        std::cout << "dec bc" << '\n';
        break;
    case 0x0c:
        inc8(regs.c);
        std::cout << "inc c" << '\n';
        break;
    case 0x0d:
        dec8(regs.c);
        std::cout << "dec c" << '\n';
        break;
    case 0x0e:
        regs.c = m_emul->read8(pc++);
        std::cout << "ld c, " << std::hex << (unsigned)regs.c << '\n';
        break;
    case 0x16:
        regs.d = m_emul->read8(pc++);
        std::cout << "ld d, " << std::hex << (unsigned)regs.d << '\n';
        break;
    case 0x19:
        std::cout << "add hl, de\n";
        add16(regs.hl, regs.de);
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
    case 0x23:
        std::cout << "inc hl\n";
        regs.hl++;
        break;
    case 0x2a:
        std::cout << "ld a, (hl+)\n";
        regs.a = m_emul->read8(regs.hl++);
        break;
    case 0x2f:
        std::cout << "cpl\n";
        regs.a ^= 0xff;
        break;
    case 0x31:
        std::cout << "ld sp, " << std::hex << (unsigned)m_emul->read16(pc);
        sp = m_emul->read16(pc);
        pc += 2;
        break;
    case 0x32:
        m_emul->write8(regs.hl--, regs.a);
        std::cout << "ld (hl-), a" << '\n';
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
    case 0x87:
        std::cout << "add a,a\n";
        add8(regs.a, regs.a);
        break;
    case 0xa1:
        std::cout << "and c\n";
        regs.a &= regs.c;
        regs.flags.z = (regs.a == 0);
        break;
    case 0xa9:
        std::cout << "xor c\n";
        regs.a ^= regs.c;
        regs.flags.z = (regs.a == 0);
        break;
    case 0xb1:
        std::cout << "or c\n";
        regs.a |= regs.c;
        regs.flags.z = (regs.a == 0);
        break;
    case 0xaf:
        regs.a = 0;
        std::cout << "xor a, a" << '\n';
        break;
    case 0xb0:
        std::cout << "or b\n";
        regs.a |= regs.b;
        regs.flags.z = (regs.a == 0);
        break;
    case 0xc3:
        pc = m_emul->read16(pc);
        std::cout << "jp " << std::hex << pc << '\n';
        break;
    case 0xc9:
        std::cout << "ret\n";
        pc = m_emul->read16(sp);
        sp += 2;
        break;
    case 0xcb:
        return exec_cb();
    case 0xcd:
        std::cout << "call " << std::hex << (unsigned)m_emul->read16(pc) << '\n';
        sp -= 2;
        m_emul->write16(sp, pc + 2);
        pc = m_emul->read16(pc);
        break;
    case 0xd5:
        std::cout << "push de\n";
        sp -= 2;
        m_emul->write16(sp, regs.de);
        break;
    case 0xe0:
        std::cout << "ldh (" << std::hex << (unsigned)m_emul->read8(pc) << "), a\n";
        m_emul->write8(0xff00 + m_emul->read8(pc++), regs.a);
        break;
    case 0xe1:
        std::cout << "pop hl\n";
        regs.hl = m_emul->read16(sp);
        sp += 2;
        break;
    case 0xe2:
        std::cout << "ld (c), a\n";
        m_emul->write8(0xff00 + regs.c, regs.a);
        break;
    case 0xe6:
        std::cout << "and " << std::hex << (unsigned)m_emul->read8(pc) << "\n";
        regs.a &= m_emul->read8(pc++);
        regs.flags.z = (regs.a == 0);
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
        sp -= 2;
        m_emul->write16(sp, pc);
        pc = 0x28;
        break;
    case 0xf0:
        std::cout << "ldh a, (" << std::hex << (unsigned)m_emul->read8(pc) << ")\n";
        regs.a = m_emul->read8(0xff00 + m_emul->read8(pc++));
        break;
    case 0xf3:
        m_interrupts_enabled = false;
        std::cout << "di" << '\n';
        break;
    case 0xfb:
        m_interrupts_enabled = true;
        std::cout << "ei" << '\n';
        break;
    case 0xfe:
        std::cout << "cp " << (unsigned)m_emul->read8(pc) << '\n';
        regs.flags.z = (regs.a == m_emul->read8(pc));
        regs.flags.c = (regs.a < m_emul->read8(pc++));
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
    default:
        std::cout << "Unimplemented CB opcode: " << std::hex << (unsigned)op << '\n';
        return false;
    }
    return true;
}
