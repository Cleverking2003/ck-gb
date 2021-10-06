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
    sf::RenderWindow window(sf::VideoMode(160, 144), "CleverKing's Gameboy Emulator", sf::Style::Default);
    while (window.isOpen()) {
        if (!Emulator::exec()) break;
        if (Emulator::elapsed_cycles() > 4194304) {
            sf::Event e;
            while(window.pollEvent(e)) {
                if (e.type == sf::Event::Closed) {
                    window.close();
                }
            }
            window.clear(sf::Color::White);
            window.draw(*Emulator::draw());
            window.display();
        }
    }
    return 0;
}
