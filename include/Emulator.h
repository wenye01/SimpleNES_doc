#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>

namespace _NES
{
    class Emulator
    {
    public:
        Emulator();
        
        void run(std::string rom_path);

        void setKeys();

    private:

    };

    Emulator::Emulator()
    {
    }

    Emulator::~Emulator()
    {
    }
}