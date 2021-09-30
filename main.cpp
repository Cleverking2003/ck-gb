#include <iostream>
#include "emul.hpp"

int main() {
    Emulator emul("tetris.gb");
    while (emul.exec());
    return 0;
}