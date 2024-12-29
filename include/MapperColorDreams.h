#pragma once
#include "Mapper.h"

namespace _NES
{
    class MapperColorDreams : public Mapper
    {
    public:
        MapperColorDreams(Cartridge &cart, std::function<void(void)> mirroring_cb);
        NameTableMirroring getNameTableMirroring();
        void writePRG(Address address, Byte value);
        Byte readPRG(Address address);

        Byte readCHR(Address address);
        void writeCHR(Address address, Byte value);

    private:
        NameTableMirroring m_mirroring;
        uint32_t prgbank;
        uint32_t chrbank;
        std::function<void(void)> m_mirroringCallback;
    };
}
