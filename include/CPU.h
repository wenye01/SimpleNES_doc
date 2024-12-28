#pragma once

#include "CPUOpencodes.h"
#include "Cartridge.h"

namespace _NES
{
    class CPU_6502
    {
    public:
        CPU_6502();

        void step();
        void reset();
        void reset(Address start_address);

        // 外部调用中断
        void interrupt(InterruptType type);
    private:
        // CPU执行中断
        void interruptSequence(InterruptType type);

        // 寄存器，pc有16bits，p有6bits，其余8bits
        Address PC;
        Byte SP;
        Byte A;
        Byte X;
        Byte Y;
        Byte P;

        // 中断类型
        bool pendingNMI;
        bool pendingIRQ;
    };
}