#include "CPU.h"

namespace _NES
{
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
}