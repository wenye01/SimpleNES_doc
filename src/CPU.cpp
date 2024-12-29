#include "CPU.h"
#include "CPUOpcodes.h"
#include "Log.h"
#include <iomanip>

namespace _NES
{
    CPU::CPU(MainBus &mem) :
        pendingNMI(false),
        pendingIRQ(false),
        bus(mem)
    {}

    void CPU::reset()
    {
        reset(readAddress(ResetVector));
    }

    void CPU::reset(Address start_addr)
    {
        m_skipCycles = m_cycles = 0;
        A = X = Y = 0;
        P.I = true;
        P.C = P.D = P.N = P.V = P.Z = false;
        PC = start_addr;
        SP = 0xfd; //documented startup state
    }

    void CPU::interrupt(InterruptType type)
    {
        switch (type)
        {
        case InterruptType::NMI_:
            pendingNMI = true;
            break;

        case InterruptType::IRQ_:
            pendingIRQ = true;
            break;

        default:
            break;
        }
    }

    void CPU::interruptSequence(InterruptType type)
    {
        if (P.I && type != NMI_ && type != BRK_)
            return;

        if (type == BRK_) //Add one if BRK, a quirk of 6502
            ++PC;

        pushStack(PC >> 8);
        pushStack(PC);

        Byte flags = P.N << 7 |
                     P.V << 6 |
                       1 << 5 | //unused bit, supposed to be always 1
          (type == BRK_) << 4 | //B flag set if BRK
                     P.D << 3 |
                     P.I << 2 |
                     P.Z << 1 |
                     P.C;
        pushStack(flags);

        P.I = true;

        switch (type)
        {
            case IRQ_:
            case BRK_:
                PC = readAddress(IRQVector);
                break;
            case NMI_:
                PC = readAddress(NMIVector);
                break;
        }

        // Interrupt sequence takes 7, but one cycle was actually spent on this.
        // So skip 6
        m_skipCycles += 6;
    }

    void CPU::pushStack(Byte value)
    {
        bus.write(0x100 | SP, value);
        --SP; //Hardware stacks grow downward!
    }

    Byte CPU::pullStack()
    {
        return bus.read(0x100 | ++SP);
    }

    void CPU::setZN(Byte value)
    {
        P.Z = !value;
        P.N = value & 0x80;
    }

    void CPU::setPageCrossed(Address a, Address b, int inc)
    {
        //Page is determined by the high byte
        if ((a & 0xff00) != (b & 0xff00))
            m_skipCycles += inc;
    }

    void CPU::skipDMACycles()
    {
        m_skipCycles += 513; //256 read + 256 write + 1 dummy read
        m_skipCycles += (m_cycles & 1); //+1 if on odd cycle
    }

    void CPU::step()
    {
        ++m_cycles;

        if (m_skipCycles-- > 1)
            return;

        m_skipCycles = 0;

        // NMI has higher priority, check for it first
        if (pendingNMI)
        {
            interruptSequence(NMI_);
            pendingNMI = pendingIRQ = false;
            return;
        }
        else if (pendingIRQ)
        {
            interruptSequence(IRQ_);
            pendingNMI = pendingIRQ = false;
            return;
        }

        int psw =    P.N << 7 |
                     P.V << 6 |
                       1 << 5 |
                     P.D << 3 |
                     P.I << 2 |
                     P.Z << 1 |
                     P.C;
        LOG_CPU << std::hex << std::setfill('0') << std::uppercase
                  << std::setw(4) << +PC
                  << "  "
                  << std::setw(2) << +bus.read(PC)
                  << "  "
                  << "A:"   << std::setw(2) << +A << " "
                  << "X:"   << std::setw(2) << +X << " "
                  << "Y:"   << std::setw(2) << +Y << " "
                  << "P:"   << std::setw(2) << psw << " "
                  << "SP:"  << std::setw(2) << +SP  << /*std::endl;*/" "
                  << "CYC:" << std::setw(3) << std::setfill(' ') << std::dec << ((m_cycles - 1) * 3) % 341
                  << std::endl;

        Byte opcode = bus.read(PC++);

        auto CycleLength = OperationCycles[opcode];

        //Using short-circuit evaluation, call the other function only if the first failed
        //ExecuteImplied must be called first and ExecuteBranch must be before ExecuteType0
        if (CycleLength && (executeImplied(opcode) || executeBranch(opcode) ||
                        executeType1(opcode) || executeType2(opcode) || executeType0(opcode)))
        {
            m_skipCycles += CycleLength;
            //m_cycles %= 340; //compatibility with Nintendulator log
            //m_skipCycles = 0; //for TESTING
        }
        else
        {
            LOG(Error) << "Unrecognized opcode: " << std::hex << +opcode << std::endl;
        }
    }

    bool CPU::executeImplied(Byte opcode)
    {
        switch (static_cast<OperationImplied>(opcode))
        {
            case NOP:
                break;
            case BRK:
                interruptSequence(BRK_);
                break;
            case JSR:
                //Push address of next instruction - 1, thus r_PC + 1 instead of r_PC + 2
                //since r_PC and r_PC + 1 are address of subroutine
                pushStack(static_cast<Byte>((PC + 1) >> 8));
                pushStack(static_cast<Byte>(PC + 1));
                PC = readAddress(PC);
                break;
            case RTS:
                PC = pullStack();
                PC |= pullStack() << 8;
                ++PC;
                break;
            case RTI:
                {
                    Byte flags = pullStack();
                    P.N = flags & 0x80;
                    P.V = flags & 0x40;
                    P.D = flags & 0x8;
                    P.I = flags & 0x4;
                    P.Z = flags & 0x2;
                    P.C = flags & 0x1;
                }
                PC = pullStack();
                PC |= pullStack() << 8;
                break;
            case JMP:
                PC = readAddress(PC);
                break;
            case JMPI:
                {
                    Address location = readAddress(PC);
                    //6502 has a bug such that the when the vector of anindirect address begins at the last byte of a page,
                    //the second byte is fetched from the beginning of that page rather than the beginning of the next
                    //Recreating here:
                    Address Page = location & 0xff00;
                    PC = bus.read(location) |
                           bus.read(Page | ((location + 1) & 0xff)) << 8;
                }
                break;
            case PHP:
                {
                    Byte flags = P.N << 7 |
                                 P.V << 6 |
                                   1 << 5 | //supposed to always be 1
                                   1 << 4 | //PHP pushes with the B flag as 1, no matter what
                                 P.D << 3 |
                                 P.I << 2 |
                                 P.Z << 1 |
                                 P.C;
                    pushStack(flags);
                }
                break;
            case PLP:
                {
                    Byte flags = pullStack();
                    P.N = flags & 0x80;
                    P.V = flags & 0x40;
                    P.D = flags & 0x8;
                    P.I = flags & 0x4;
                    P.Z = flags & 0x2;
                    P.C = flags & 0x1;
                }
                break;
            case PHA:
                pushStack(A);
                break;
            case PLA:
                A = pullStack();
                setZN(A);
                break;
            case DEY:
                --Y;
                setZN(Y);
                break;
            case DEX:
                --X;
                setZN(X);
                break;
            case TAY:
                Y = A;
                setZN(Y);
                break;
            case INY:
                ++Y;
                setZN(Y);
                break;
            case INX:
                ++X;
                setZN(X);
                break;
            case CLC:
                P.C = false;
                break;
            case SEC:
                P.C = true;
                break;
            case CLI:
                P.I = false;
                break;
            case SEI:
                P.I = true;
                break;
            case CLD:
                P.D = false;
                break;
            case SED:
                P.D = true;
                break;
            case TYA:
                A = Y;
                setZN(A);
                break;
            case CLV:
                P.V = false;
                break;
            case TXA:
                A = X;
                setZN(A);
                break;
            case TXS:
                SP = X;
                break;
            case TAX:
                X = A;
                setZN(X);
                break;
            case TSX:
                X = SP;
                setZN(X);
                break;
            default:
                return false;
        };
        return true;
    }

    bool CPU::executeBranch(Byte opcode)
    {
        if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult)
        {
            //branch is initialized to the condition required (for the flag specified later)
            bool branch = opcode & BranchConditionMask;

            //set branch to true if the given condition is met by the given flag
            //We use xnor here, it is true if either both operands are true or false
            switch (opcode >> BranchOnFlagShift)
            {
                case Negative:
                    branch = !(branch ^ P.N);
                    break;
                case Overflow:
                    branch = !(branch ^ P.V);
                    break;
                case Carry:
                    branch = !(branch ^ P.C);
                    break;
                case Zero:
                    branch = !(branch ^ P.Z);
                    break;
                default:
                    return false;
            }

            if (branch)
            {
                int8_t offset = bus.read(PC++);
                ++m_skipCycles;
                auto newPC = static_cast<Address>(PC + offset);
                setPageCrossed(PC, newPC, 2);
                PC = newPC;
            }
            else
                ++PC;
            return true;
        }
        return false;
    }

    bool CPU::executeType1(Byte opcode)
    {
        if ((opcode & InstructionModeMask) == 0x1)
        {
            Address location = 0; //Location of the operand, could be in RAM
            auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);
            switch (static_cast<AddrMode1>(
                    (opcode & AddrModeMask) >> AddrModeShift))
            {
                case IndexedIndirectX:
                    {
                        Byte zero_addr = X + bus.read(PC++);
                        //Addresses wrap in zero page mode, thus pass through a mask
                        location = bus.read(zero_addr & 0xff) | bus.read((zero_addr + 1) & 0xff) << 8;
                    }
                    break;
                case ZeroPage:
                    location = bus.read(PC++);
                    break;
                case Immediate:
                    location = PC++;
                    break;
                case Absolute:
                    location = readAddress(PC);
                    PC += 2;
                    break;
                case IndirectY:
                    {
                        Byte zero_addr = bus.read(PC++);
                        location = bus.read(zero_addr & 0xff) | bus.read((zero_addr + 1) & 0xff) << 8;
                        if (op != STA)
                            setPageCrossed(location, location + Y);
                        location += Y;
                    }
                    break;
                case IndexedX:
                    // Address wraps around in the zero page
                    location = (bus.read(PC++) + X) & 0xff;
                    break;
                case AbsoluteY:
                    location = readAddress(PC);
                    PC += 2;
                    if (op != STA)
                        setPageCrossed(location, location + Y);
                    location += Y;
                    break;
                case AbsoluteX:
                    location = readAddress(PC);
                    PC += 2;
                    if (op != STA)
                        setPageCrossed(location, location + X);
                    location += X;
                    break;
                default:
                    return false;
            }

            switch (op)
            {
                case ORA:
                    A |= bus.read(location);
                    setZN(A);
                    break;
                case AND:
                    A &= bus.read(location);
                    setZN(A);
                    break;
                case EOR:
                    A ^= bus.read(location);
                    setZN(A);
                    break;
                case ADC:
                    {
                        Byte operand = bus.read(location);
                        std::uint16_t sum = A + operand + P.C;
                        //Carry forward or UNSIGNED overflow
                        P.C = sum & 0x100;
                        //SIGNED overflow, would only happen if the sign of sum is
                        //different from BOTH the operands
                        P.V = (A ^ sum) & (operand ^ sum) & 0x80;
                        A = static_cast<Byte>(sum);
                        setZN(A);
                    }
                    break;
                case STA:
                    bus.write(location, A);
                    break;
                case LDA:
                    A = bus.read(location);
                    setZN(A);
                    break;
                case SBC:
                    {
                        //High carry means "no borrow", thus negate and subtract
                        std::uint16_t subtrahend = bus.read(location),
                                 diff = A - subtrahend - !P.C;
                        //if the ninth bit is 1, the resulting number is negative => borrow => low carry
                        P.C = !(diff & 0x100);
                        //Same as ADC, except instead of the subtrahend,
                        //substitute with it's one complement
                        P.V = (A ^ diff) & (~subtrahend ^ diff) & 0x80;
                        A = diff;
                        setZN(diff);
                    }
                    break;
                case CMP:
                    {
                        std::uint16_t diff = A - bus.read(location);
                        P.C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::executeType2(Byte opcode)
    {
        if ((opcode & InstructionModeMask) == 2)
        {
            Address location = 0;
            auto op = static_cast<Operation2>((opcode & OperationMask) >> OperationShift);
            auto addr_mode =
                    static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift);
            switch (addr_mode)
            {
                case Immediate_:
                    location = PC++;
                    break;
                case ZeroPage_:
                    location = bus.read(PC++);
                    break;
                case Accumulator:
                    break;
                case Absolute_:
                    location = readAddress(PC);
                    PC += 2;
                    break;
                case Indexed:
                    {
                        location = bus.read(PC++);
                        Byte index;
                        if (op == LDX || op == STX)
                            index = Y;
                        else
                            index = X;
                        //The mask wraps address around zero page
                        location = (location + index) & 0xff;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        location = readAddress(PC);
                        PC += 2;
                        Byte index;
                        if (op == LDX || op == STX)
                            index = Y;
                        else
                            index = X;
                        setPageCrossed(location, location + index);
                        location += index;
                    }
                    break;
                default:
                    return false;
            }

            std::uint16_t operand = 0;
            switch (op)
            {
                case ASL:
                case ROL:
                    if (addr_mode == Accumulator)
                    {
                        auto prev_C = P.C;
                        P.C = A & 0x80;
                        A <<= 1;
                        //If Rotating, set the bit-0 to the the previous carry
                        A = A | (prev_C && (op == ROL));
                        setZN(A);
                    }
                    else
                    {
                        auto prev_C = P.C;
                        operand = bus.read(location);
                        P.C = operand & 0x80;
                        operand = operand << 1 | (prev_C && (op == ROL));
                        setZN(operand);
                        bus.write(location, operand);
                    }
                    break;
                case LSR:
                case ROR:
                    if (addr_mode == Accumulator)
                    {
                        auto prev_C = P.C;
                        P.C = A & 1;
                        A >>= 1;
                        //If Rotating, set the bit-7 to the previous carry
                        A = A | (prev_C && (op == ROR)) << 7;
                        setZN(A);
                    }
                    else
                    {
                        auto prev_C = P.C;
                        operand = bus.read(location);
                        P.C = operand & 1;
                        operand = operand >> 1 | (prev_C && (op == ROR)) << 7;
                        setZN(operand);
                        bus.write(location, operand);
                    }
                    break;
                case STX:
                    bus.write(location, X);
                    break;
                case LDX:
                    X = bus.read(location);
                    setZN(X);
                    break;
                case DEC:
                    {
                        auto tmp = bus.read(location) - 1;
                        setZN(tmp);
                        bus.write(location, tmp);
                    }
                    break;
                case INC:
                    {
                        auto tmp = bus.read(location) + 1;
                        setZN(tmp);
                        bus.write(location, tmp);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::executeType0(Byte opcode)
    {
        if ((opcode & InstructionModeMask) == 0x0)
        {
            Address location = 0;
            switch (static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift))
            {
                case Immediate_:
                    location = PC++;
                    break;
                case ZeroPage_:
                    location = bus.read(PC++);
                    break;
                case Absolute_:
                    location = readAddress(PC);
                    PC += 2;
                    break;
                case Indexed:
                    // Address wraps around in the zero page
                    location = (bus.read(PC++) + X) & 0xff;
                    break;
                case AbsoluteIndexed:
                    location = readAddress(PC);
                    PC += 2;
                    setPageCrossed(location, location + X);
                    location += X;
                    break;
                default:
                    return false;
            }
            std::uint16_t operand = 0;
            switch (static_cast<Operation0>((opcode & OperationMask) >> OperationShift))
            {
                case BIT:
                    operand = bus.read(location);
                    P.Z = !(A & operand);
                    P.V = operand & 0x40;
                    P.N = operand & 0x80;
                    break;
                case STY:
                    bus.write(location, Y);
                    break;
                case LDY:
                    Y = bus.read(location);
                    setZN(Y);
                    break;
                case CPY:
                    {
                        std::uint16_t diff = Y - bus.read(location);
                        P.C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                case CPX:
                    {
                        std::uint16_t diff = X - bus.read(location);
                        P.C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }
        return false;
    }

    Address CPU::readAddress(Address addr)
    {
        return bus.read(addr) | bus.read(addr + 1) << 8;
    }

};

