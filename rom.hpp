#pragma once

#include "file.hpp"

class ROM {
public:
    bool load(const char* game);
    void unload() { delete m_rom; delete m_exram; }
    unsigned char read8(unsigned short addr);
    void write8(unsigned short addr, unsigned char val);
    int get_bank() { return m_current_bank; }
    ~ROM() { unload(); }
private:
    void mbc1_write(unsigned short addr, unsigned char val);
    void mbc3_write(unsigned short addr, unsigned char val);
    void mbc5_write(unsigned short addr, unsigned char val);

    unsigned char* m_rom { nullptr };
    unsigned char* m_exram { nullptr };
    int m_size { 0 };
    int m_exram_size { 0 };
    int m_mapper_type { 0 };
    int m_current_bank { 1 };
    int m_exram_bank { 0 };

    // MBC1
    bool m_mode_select { false };
    bool m_enable_exram { false };
};
