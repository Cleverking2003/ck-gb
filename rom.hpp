#pragma once

#include "file.hpp"

class ROM {
public:
    bool load(const char* game);
    void unload() { delete m_rom; }
    unsigned char read8(unsigned short addr);
    void write8(unsigned short addr, unsigned char val);
    int get_bank() { return m_current_bank; }
    ~ROM() { unload(); }
private:
    void mbc1_write(unsigned short addr, unsigned char val);

    unsigned char* m_rom { nullptr };
    int m_size { 0 };
    int m_mapper_type { 0 };
    int m_current_bank { 1 };

    // MBC1
    bool m_mode_select { false };
};
