#include <iostream>
#include "emul.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ck-gb <filename>\n";
        return 1;
    }
    Emulator::the();
    if (!Emulator::load(argv[1])) {
        std::cout << "Couldn't load the game\n";
        return 1;
    }
    while (Emulator::exec());
    return 0;
}