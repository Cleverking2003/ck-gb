#include "file.hpp"

bool File::open(const char* filename, const char* mode) {
    m_file = fopen(filename, mode);
    if (m_file) {
        fseek(m_file, 0, SEEK_END);
        m_size = ftell(m_file);
        fseek(m_file, 0, SEEK_SET);
    }
    return m_file != nullptr;
}

int File::read(char* buffer, int count) {
    return fread(buffer, sizeof(char), count, m_file);
}

void File::seek(int pos) {
    m_pos = pos;
    fseek(m_file, m_pos, SEEK_SET);
}

void File::close() {
    fclose(m_file);
    m_file = nullptr;
}
