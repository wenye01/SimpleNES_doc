#include "MainBus.h"
#include "Log.h"
#include <cstring>

namespace _NES
{
    MainBus::MainBus()
        :RAM(0x800, 0)
    {

    }

    Byte MainBus::read(Address address)
    {
        if (address < 0x2000) // 内存
        {
            // 因为有三份数据镜像，只取0x7ff以内数据
            return RAM[address & 0x7ff]; 
        }
        else if (address < 0x4020)
        {
            if (address < 0x4000) // PPU寄存器
            {
                auto it = readCallbacks.find(static_cast<IORegisters>(address % 0x2007));
                if (it != readCallbacks.end())
                {
                    return (it->second)();
                }
                else
                {
                    LOG(InfoVerbose, "No read callback registered for I / O register at: " + std::to_string(address));
                }
            }
            else if (address < 0x4018 && address >= 0x4014)
            {
                auto it = readCallbacks.find(static_cast<IORegisters>(address));
                if (it != readCallbacks.end())
                {
                    return (it->second)();
                }
                else
                {
                    LOG(InfoVerbose, "No read callback registered for I / O register at: " + std::to_string(address));
                }
            }
            else
            {
                LOG(InfoVerbose, "Read access attempt at: " + std::to_string(address));
            }
        }
        else if (address < 0x6000)
        {
            LOG(InfoVerbose, "Expansion ROM read attempted. This is currently unsupported");
        }
        else if (address < 0x8000)
        {
            // 卡带上的额外内存
        }
        else
        {
            // mapper
        }
        return 0;
    }

    void MainBus::write(Address address, Byte value)
    {
        if (address < 0x2000)
        {
            RAM[address & 0x7fff] = value;
        }
        else if (address < 0x4020)
        {
            if (address < 0x4000)
            {
                auto it = readCallbacks.find(static_cast<IORegisters>(address % 0x2007));
                if (it != readCallbacks.end())
                {
                    (it->second)();
                }
                else
                {
                    LOG(InfoVerbose, "No write callback registered for I / O register at: " + std::to_string(address));
                }
            }
            else if (address < 0x4017 && address >= 0x4014)// ?
            {
                auto it = readCallbacks.find(static_cast<IORegisters>(address));
                if (it != readCallbacks.end())
                {
                    (it->second)();
                }
                else
                {
                    LOG(InfoVerbose, "No write callback registered for I / O register at: " + std::to_string(address));
                }
            }
            else
            {
                LOG(InfoVerbose, "Write access attempt at: " + std::to_string(address));
            }
        }
        else if (address < 0x6000)
        {
            LOG(InfoVerbose, "Expansion ROM read attempted. This is currently unsupported");
        }
        else if (address < 0x8000)
        {
            // 卡带上的额外内存
        }
        else
        {
            // mapper
        }
    }

}