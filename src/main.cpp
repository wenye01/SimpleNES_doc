#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>

#include "Log.h"

int main(int argc, char** argv)
{
    std::ofstream logFile("simplenes.log"), cpuTraceFile;
    _NES::TeeStream logTee(logFile, std::cout);

    if (logFile.is_open() && logFile.good())
        _NES::Log::get().setLogStream(logTee);
    else
        _NES::Log::get().setLogStream(std::cout);

    _NES::Log::get().setLevel(_NES::Info);

    std::string path;
    std::string keybindingsPath = "keybindings.conf";

    std::vector<sf::Keyboard::Key>  
        p1{ sf::Keyboard::J,        sf::Keyboard::K,        sf::Keyboard::RShift,   sf::Keyboard::Return,
            sf::Keyboard::W,        sf::Keyboard::S,        sf::Keyboard::A,        sf::Keyboard::D },
        p2{ sf::Keyboard::Numpad5,  sf::Keyboard::Numpad6,  sf::Keyboard::Numpad8,  sf::Keyboard::Numpad9,
            sf::Keyboard::Up,       sf::Keyboard::Down,     sf::Keyboard::Left,     sf::Keyboard::Right };


    std::cout << "test";
    return 0;
}