#include <iostream>
#include "emul.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ck-gb <filename>\n";
        return 1;
    }
    Emulator emul(argv[1]);
    while (emul.exec());
    return 0;
}