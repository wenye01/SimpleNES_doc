#pragma once
#include "Mapper.h"

namespace _NES
{
    class MapperNROM : public Mapper
    {
        public:
            MapperNROM(Cartridge& cart);
            void writePRG (Address addr, Byte value);
            Byte readPRG (Address addr);

            Byte readCHR (Address addr);
            void writeCHR (Address addr, Byte value);
        private:
            bool m_oneBank;
            bool m_usesCharacterRAM;

            std::vector<Byte> m_characterRAM;

    };
}
