#include "Cartridge.h"
#include "Log.h"

namespace _NES
{
    Cartridge::Cartridge()
        :m_nameTableMirroring(0),
        m_mapperNumber(0),
        m_extendedRAM(false)
    {

    }
}