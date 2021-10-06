#include <iostream>
#include <SFML/Graphics.hpp>
#include "emul.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ck-gb <filename>\n";
        return 1;
    }
    Emulator::create();
    if (!Emulator::load(argv[1])) {
        std::cout << "Couldn't load the game\n";
        return 1;
    }
    Emulator::run();
    return 0;
}
