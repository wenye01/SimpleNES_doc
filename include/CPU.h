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

        // �ⲿ�����ж�
        void interrupt(InterruptType type);
    private:
        // CPUִ���ж�
        void interruptSequence(InterruptType type);

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
    };
}