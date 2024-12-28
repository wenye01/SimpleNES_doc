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

        // ?Ӧ���Ǵ�PPU��ȡ���ĵ�����
        void skipDMACycles();

        // �ⲿ�����ж�
        void interrupt(InterruptType type);
    private:
        // CPUִ���ж�
        void interruptSequence(InterruptType type);

        void pushStack(Byte value);
        Byte pullStack();

        Address readAddress(Address address);
        
    private:

        int skipCycles;
        int cycles;

        // �Ĵ�����pc��16bits��p��6bits������8bits
        Address PC;
        Byte SP;
        Byte A;
        Byte X;
        Byte Y;
        Byte P;

        // �ж�����
        bool pendingNMI;
        bool pendingIRQ;

        MainBus& bus;
    };
}