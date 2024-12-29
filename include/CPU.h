#pragma once
#include "CPUOpcodes.h"
#include "MainBus.h"

namespace _NES
{

    class CPU
    {
        public:

            CPU(MainBus &mem);

            void step();
            void reset();
            void reset(Address start_addr);
            void log();

            Address getPC() { return PC; }
            void skipDMACycles();

            void interrupt(InterruptType type);

        private:
            void interruptSequence(InterruptType type);

            //Instructions are split into five sets to make decoding easier.
            //These functions return true if they succeed
            bool executeImplied(Byte opcode);
            bool executeBranch(Byte opcode);
            bool executeType0(Byte opcode);
            bool executeType1(Byte opcode);
            bool executeType2(Byte opcode);

            Address readAddress(Address addr);

            void pushStack(Byte value);
            Byte pullStack();

            //If a and b are in different pages, increases the m_SkipCycles by inc
            void setPageCrossed(Address a, Address b, int inc = 1);
            void setZN(Byte value);

            int m_skipCycles;
            int m_cycles;

            //Registers
            Address PC;
            Byte SP;
            Byte A;
            Byte X;
            Byte Y;

            struct
            {
                Byte C : 1;
                Byte Z : 1;
                Byte I : 1;
                Byte D : 1;
                Byte   : 2;
                Byte V : 1;
                Byte N : 1;
            }P;

            //Status flags.
            //Is storing them in one byte better ?
            bool f_C;
            bool f_Z;
            bool f_I;
            bool f_D;
            bool f_V;
            bool f_N;

            bool pendingNMI;
            bool pendingIRQ;

            MainBus &bus;
    };

};
