#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include "Cartridge.h"

namespace _NES
{
    enum IORegisters
    {
        PPUCTRL = 0x2000,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROL,
        PPUADDR,
        PPUDATA,
        OAMDMA = 0x4014,
        JOY1 = 0x4016,
        JOY2 = 0x4017,
    };

    struct IORegistersHasher
    {
        std::size_t operator()(_NES::IORegisters const& reg) const noexcept
        {
            return std::hash<std::uint32_t>{}(reg);
        }
    };

    class MainBus
    {
    public:
        MainBus();

        Byte read(Address address);
        void write(Address, Byte value);

        bool setWriteCallback(IORegisters registe, std::function<void(Byte)> callback);
        bool setReadCallback(IORegisters registe, std::function<Byte(void)> callback);


    private:
        std::vector<Byte> RAM;
        std::vector<Byte> extRAM;

        std::unordered_map<IORegisters, std::function<void(Byte)>, IORegistersHasher> writeCallbacks;
        std::unordered_map<IORegisters, std::function<Byte(void)>, IORegistersHasher> readCallbacks;;

    };
}