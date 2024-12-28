#include "CPU.h"

namespace _NES
{
    CPU_6502::CPU_6502(MainBus& mainBus)
        :pendingNMI(false), pendingIRQ(false),
        bus(mainBus)
    {}

    void CPU_6502::step()
    {

    }

    void CPU_6502::reset()
    {
        reset(readAddress(ResetVector));
    }

    void CPU_6502::reset(Address start_address)
    {
        skipCycles  = 0;
        cycles = 0;

        A = 0;
        X = 0;
        Y = 0;

        // ?
        P = 1 << 2;

        PC = start_address;
        SP = 0xfd;
    }

    // ?
    void CPU_6502::skipDMACycles()
    {
        skipCycles += 513;
        skipCycles += (cycles & 1);
    }

    void CPU_6502::interrupt(InterruptType type)
    {
        switch (type)
        {
        case InterruptType::NMI:
            pendingNMI = true;
            break;

        case InterruptType::IRQ:
            pendingIRQ = true;
            break;

        default:
            break;
        }
    }

    void CPU_6502::pushStack(Byte value)
    {
        bus.write(0x100 | SP, value);
        --SP;
    }

    Byte CPU_6502::pullStack()
    {
        ++SP;
        return bus.read(0x100 | SP);
    }

    // 低8位和高8位
    Address CPU_6502::readAddress(Address address)
    {
        return bus.read(address) | bus.read(address + 1) << 8;
    }

}