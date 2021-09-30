#pragma once

#include <cstdio>
#include <string>

using namespace std;

class File {
public:
    bool open(const char* filename, const char* mode);
    int read(char* buffer, int count);
    int pos() { return m_pos; }
    void seek(int pos);
    int size() { return m_size; };
    void close();
    ~File() { if (m_file) close(); }
private:
    FILE* m_file { nullptr };
    int m_size { 0 };
    int m_pos { 0 };
};
