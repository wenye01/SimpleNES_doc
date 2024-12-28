#pragma once

#include "CPUOpencodes.h"
#include "MainBus.h"

namespace _NES
{
    class CPU_6502
    {
    public:
        CPU_6502(MainBus& mainBus);

        void step();
        void reset();
        void reset(Address start_address);

        Address getPC() { return PC; }

        // ?应该是从PPU读取消耗的周期
        void skipDMACycles();

        // 外部调用中断
        void interrupt(InterruptType type);
    private:
        // CPU执行中断
        void interruptSequence(InterruptType type);

        void pushStack(Byte value);
        Byte pullStack();

        Address readAddress(Address address);
        
    private:

        int skipCycles;
        int cycles;

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

        MainBus& bus;
    };
}