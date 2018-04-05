/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 26, 2018
 * Description:
 *      Simulator for M20 Processor. (Implementation)
 * =============================================================================
 */

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "Simulator.h"

void m20::Simulator::load(const std::string &fname)
{
    std::ifstream infile(fname, std::ios::ate | std::ios::binary);
    assert(infile.is_open());
    int64_t size = infile.tellg();
    infile.seekg(0);
    infile.read(mem, size);
    infile.close();
}

void m20::Simulator::simulate()
{
    static const int DATA_SIGNATURE = 0x08000000;
    static const int LOAD_SIGNATURE = 0x04000000;
    static const int BRANCH_SIGNATURE = 0x02000000;
    static const int COPROC_SIGNATURE = 0x01000000;

    // Initialize simulator
    reg_pc = 0;                     // Set to first instruction
    reg_st = Simulator::MODE_SVR;   // Set to supervisor mode
    for (int i = 0; i <= 12; ++i)   // Zero out registers
    {
        *getRegister(i) = 0;
    }
    *getRegister(13) = 0xfff8;      // Set stack ptr
    *getRegister(14) = 0xfffc;      // Set link ptr to halt handler
    storeWord(0xfffc, 0xE1F00000);  // Create halt handler
    halt = false;

    while (!halt)
    {
        try
        {
            if (!(reg_pc >= 0 && reg_pc < MAX_ADDRESS))
            {
                throw PrefetchAbortException();
            }

            int instr = loadWord(reg_pc);
            reg_pc += 4;
            if (isCondition(instr))
            {
                if (!(DATA_SIGNATURE & instr))
                {
                    simulateData(instr);
                }
                else if (!(LOAD_SIGNATURE & instr))
                {
                    simulateLoad(instr);
                }
                else if (!(BRANCH_SIGNATURE & instr))
                {
                    simulateBranch(instr);
                }
                else if (!(COPROC_SIGNATURE & instr))
                {
                    throw UsageAbortException();
                }
                else // SWI
                {
                    simulateSwi(instr);
                }
            }
        }
        catch (const UndefinedInstructionException &e)
        {
            std::cout << ">>>>> Undefined Instruction @ 0x"
                      << std::hex << reg_pc - 4 << std::endl;
            printStatus();
            halt = true;
            break;
        }
        catch (const PrefetchAbortException &e)
        {
            std::cout << ">>>>> Prefetch Abort @ 0x"
                      << std::hex << reg_pc - 4 << std::endl;
            halt = true;
            break;
        }
        catch (const DataAbortException &e)
        {
            std::cout << ">>>>> Data Abort @ 0x"
                      << std::hex << reg_pc - 4 << std::endl;
            halt = true;
            break;
        }
        catch (const UsageAbortException &e)
        {
            std::cout << ">>>>> Usage Abort @ 0x"
                      << std::hex << reg_pc - 4 << std::endl;
            halt = true;
            break;
        }
        catch (const SoftwareInterruptException &e)
        {
            std::cout << ">>>>> Software Interrupt @ 0x"
                      << std::hex << reg_pc - 4 << std::endl;

            int stream = *getRegister(0);
            auto str = (unsigned) *getRegister(1);
            int len = *getRegister(2);
            std::cout << "write(" << std::dec << stream
                      << ", 0x" << std::hex << str
                      << ", " << len << ")" << std::endl;
            for (int i = 0; i < len; ++i)
            {
                std::cout << mem[str + i];
            }
            std::cout << std::endl;
        }
        catch (...)
        {
            std::cout << ">>>>> Undefined Interrupt Vector" << std::endl;
            halt = true;
            break;
        }

        ++instructionsExecuted;
    }

    printStatus();
    std::cout << ">>>>> HALTED <<<<<" << std::endl;
}

void m20::Simulator::printStatus()
{
    std::cout << "Executed " << std::dec << instructionsExecuted
              << " instructions" << std::endl;
    std::cout << "Core Dump ----------------------\n";
    std::cout << "R0 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(0) << "\n";
    std::cout << "R1 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(1) << "\n";
    std::cout << "R2 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(2) << "\n";
    std::cout << "R3 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(3) << "\n";
    std::cout << "R4 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(4) << "\n";
    std::cout << "R5 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(5) << "\n";
    std::cout << "R6 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(6) << "\n";
    std::cout << "R7 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(7) << "\n";
    std::cout << "R8 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(8) << "\n";
    std::cout << "R9 : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(9) << "\n";
    std::cout << "R10: " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(10) << "\n";
    std::cout << "R11: " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(11) << "\n";
    std::cout << "R12: " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(12) << "\n";
    std::cout << "SP : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(13) << "\n";
    std::cout << "LP : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(14) << "\n";
    std::cout << "PC : " << std::setw(8) << std::setfill('0') << std::hex
              << *getRegister(15) << "\n";
    std::cout << "ST : " << std::setw(8) << std::setfill('0') << std::hex
              << *getStatus(0) << "\n";
    std::cout << "--------------------------------" << std::dec << std::endl;
}

bool m20::Simulator::isCondition(int instr)
{
    switch ((instr & 0xF0000000) >> 28)
    {
        case 0x0:   // EQ
            return (reg_st & ST_Z) != 0;
        case 0x1:   // NE
            return (reg_st & ST_Z) == 0;
        case 0x2:   // CS
            return (reg_st & ST_C) != 0;
        case 0x3:   // CC
            return (reg_st & ST_C) == 0;
        case 0x4:   // MI
            return (reg_st & ST_N) != 0;
        case 0x5:   // PL
            return (reg_st & ST_N) == 0;
        case 0x6:   // VS
            return (reg_st & ST_V) != 0;
        case 0x7:   // VC
            return (reg_st & ST_V) == 0;
        case 0x8:   // HI
            return (reg_st & ST_C) != 0 && (reg_st & ST_Z) == 0;
        case 0x9:   // LS
            return (reg_st & ST_C) == 0 || (reg_st & ST_Z) != 0;
        case 0xA:   // GE
            return ((reg_st & ST_N) >> 3) == (reg_st & ST_V);
        case 0xB:   // LT
            return ((reg_st & ST_N) >> 3) != (reg_st & ST_V);
        case 0xC:   // GT
            return (reg_st & ST_Z) == 0
                   && ((reg_st & ST_N) >> 3) == (reg_st & ST_V);
        case 0xD:   // LE
            return (reg_st & ST_Z) != 0
                   || ((reg_st & ST_N) >> 3) != (reg_st & ST_V);
        case 0xE:   // AL
            return true;
        default:    // INVALID
            throw UndefinedInstructionException();
            return false;
    }
}

void m20::Simulator::simulateData(int instr)
{
    static const int IMMEDIATE = 0x02000000;
    static const int UPDATE_ST = 0x04000000;
    static const int OPCODE = 0x01F00000;
    static const int RD = 0x000F0000;
    static const int RN = 0x0000F000;
    static const int IMMEDIATE_20 = 0x000FFFFF;
    static const int IMMEDIATE_20_SE = 0xFFF00000;
    static const int IMMEDIATE_16 = 0x0000FFFF;
    static const int IMMEDIATE_16_SE = 0xFFFF0000;
    static const int IMMEDIATE_12 = 0x00000FFF;
    static const int IMMEDIATE_12_SE = 0xFFFFF000;

    bool hasImmediate = (instr & IMMEDIATE) != 0;
    bool shouldUpdate = (instr & UPDATE_ST) != 0;
    int opcode = ((instr & OPCODE) >> 20) & 0x1F;
    int rd = ((instr & RD) >> 16) & 0xF;
    int rn = ((instr & RN) >> 12) & 0xF;
    int immediate20 = instr & IMMEDIATE_20;
    int immediate16 = instr & IMMEDIATE_16;
    int immediate12 = instr & IMMEDIATE_12;

    if ((immediate20 & 0x00080000) != 0)
    {
        immediate20 |= IMMEDIATE_20_SE;
    }
    if ((immediate16 & 0x00008000) != 0)
    {
        immediate16 |= IMMEDIATE_16_SE;
    }
    if ((immediate12 & 0x00000800) != 0)
    {
        immediate12 |= IMMEDIATE_12_SE;
    }

    long long aluReg = 0;
    int aluA = 0;
    int aluB = 0;

    switch (opcode)
    {
        case 0x00:  // NOOP
            throw UndefinedInstructionException();
            break;
        case 0x01:  // ADD
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA + aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x02:  // ADC
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA + aluB + ((reg_st & ST_C) != 0 ? 1 : 0);
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x03:  // SUB
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA - aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x04:  // SBC
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA - aluB - ((reg_st & ST_C) == 0 ? 1 : 0);
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x05:  // MUL
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA * aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x06:  // DIV
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA / aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x07:  // UDV
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = (int) (((size_t) aluA & 0xFFFFFFFF)
                            / ((size_t) aluB & 0xFFF));
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x08:  // OR
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA | aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x09:  // AND
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA & aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x0A:  // XOR
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA ^ aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x0B:  // NOR
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = ~(aluA | aluB);
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x0C:  // BIC
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA & ~aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x0D:  // ROR
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = (aluA >> (aluB % 32)) | (aluA << (32 - (aluB % 32)));
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x0E:  // LSL
            aluA = *getRegister(rn);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA << aluB;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x0F:  // LSR
            throw UsageAbortException();
            break;
        case 0x10:  // ASR
            throw UsageAbortException();
            break;
        case 0x11:  // MOV
            aluA = (hasImmediate ? immediate16 : *getRegister(immediate16));
            aluB = 0;
            aluReg = aluA;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x12:  // MVN
            aluA = (hasImmediate ? immediate16 : *getRegister(immediate16));
            aluB = 0;
            aluReg = ~aluA;
            *getRegister(rd) = (int) aluReg;
            break;
        case 0x13:  // CMP
            aluA = *getRegister(rd);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA - aluB;
            shouldUpdate = true;
            break;
        case 0x14:  // CMN
            aluA = *getRegister(rd);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA + aluB;
            shouldUpdate = true;
            break;
        case 0x15:  // TST
            aluA = *getRegister(rd);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA & aluB;
            shouldUpdate = true;
            break;
        case 0x16:  // TEQ
            aluA = *getRegister(rd);
            aluB = (hasImmediate ? immediate12 : *getRegister(immediate12));
            aluReg = aluA ^ aluB;
            shouldUpdate = true;
            break;
        case 0x17:  // PUSH
            *getRegister(13) -= 4;
            if (hasImmediate)
            {
                storeWord(*getRegister(13), immediate20);
            }
            else
            {
                storeWord(*getRegister(13), *getRegister(immediate20));
            }
            break;
        case 0x18:  // POP
            if (hasImmediate)
            {
                throw UndefinedInstructionException();
            }
            else
            {
                *getRegister(immediate20) = loadWord(*getRegister(13));
            }
            *getRegister(13) += 4;
            break;
        case 0x19:  // SRL
            throw UsageAbortException();
            break;
        case 0x1A:  // SRS
            throw UsageAbortException();
            break;
        case 0x1F:
            if (getMode() == 0)
            {
                throw UsageAbortException();
            }
            halt = true;
            break;
        default:
            throw UndefinedInstructionException();
    }

    if (shouldUpdate)
    {
        static const long long MASK = 0xFFFFFFFF;
        static const long long NEGATIVE = 0x0000000080000000;
        static const long long CARRY = 0xFFFFFFFF00000000;
        static const int OVERFLOW = 0x80000000;

        int status = 0;
        status |= (aluReg & NEGATIVE) == NEGATIVE ? 0x80000000 : 0x0;
        status |= (aluReg & MASK) == 0 ? 0x40000000 : 0x0;
        status |= (aluReg & CARRY) != 0 ? 0x20000000 : 0x0;
        bool carryIn = (aluA & OVERFLOW) == OVERFLOW
                       && (aluB & OVERFLOW) == OVERFLOW;
        bool carryOut = (aluReg & NEGATIVE) == NEGATIVE;
        status |= carryIn ^ carryOut;
        *getStatus(0) &= 0x0FFFFFFF;
        *getStatus(0) |= status;
    }
}

void m20::Simulator::simulateLoad(int instr)
{
    static const int IMMEDIATE = 0x02000000;
    static const int BASE = 0x01000000;
    static const int OPCODE = 0x00700000;
    static const int RD = 0x000F0000;
    static const int RN = 0x0000F000;
    static const int IMMEDIATE_16 = 0x0000FFFF;
    static const int IMMEDIATE_12 = 0x00000FFF;
    static const int IMMEDIATE_16_SE = 0xFFFF0000;
    static const int IMMEDIATE_12_SE = 0xFFFFF000;

    bool hasImmediate = (instr & IMMEDIATE) != 0;
    bool hasBase = (instr & BASE) != 0;
    int opcode = (instr & OPCODE) >> 20 & 0x7;
    int rd = ((instr & RD) >> 16) & 0xF;
    int rn = ((instr & RN) >> 12) & 0xF;
    int immediate16 = instr & IMMEDIATE_16;
    int immediate12 = instr & IMMEDIATE_12;

    int base = 0;
    int offset = 0;

    if (hasBase)
    {
        base = *getRegister(rn);
    }
    else
    {
        base = 0;
    }

    if (hasImmediate)
    {
        offset = hasBase ? immediate12 : immediate16;
        if ((hasBase && (0x00000800 & immediate12) != 0)
                || (!hasBase && (0x00008000 & immediate16) != 0))
        {
            offset |= hasBase ? IMMEDIATE_12_SE : IMMEDIATE_16_SE;
        }
    }
    else
    {
        offset = *getRegister(immediate12);
    }

    if (!hasBase && hasImmediate)
    {
        base = *getRegister(15);
    }

    switch (opcode)
    {
        case 0x0:   // LDR
            *getRegister(rd) = loadWord(base + offset);
            break;
        case 0x1:   // LDRB
            *getRegister(rd) = (int) ((unsigned int) loadByte(base + offset));
            break;
        case 0x2:   // LDRH
            *getRegister(rd) = (int) ((unsigned int)
                                     loadHalfword(base + offset));
            break;
        case 0x3:   // LDRSB
            *getRegister(rd) = loadByte(base + offset);
            break;
        case 0x4:   // LDRSH
            *getRegister(rd) = loadHalfword(base + offset);
            break;
        case 0x5:   // STR
            storeWord(base + offset, *getRegister(rd));
            break;
        case 0x6:   // STRB
            storeByte(base + offset, *getRegister(rd));
            break;
        case 0x7:   // STRH
            storeHalfword(base + offset, *getRegister(rd));
            break;
        default:
            throw UndefinedInstructionException();
    }
}

void m20::Simulator::simulateBranch(int instr)
{
    static const int IMMEDIATE = 0x00800000;
    static const int LINK = 0x01000000;
    static const int IMMEDIATE_23 = 0x007FFFFF;
    static const int IMMEDIATE_23_SE = 0xFF800000;
    static const int REGISTER = 0x0000000F;

    bool hasImmediate = (instr & IMMEDIATE) != 0;
    bool hasLink = (instr & LINK) != 0;
    int addr = instr & IMMEDIATE_23;
    int reg = instr & REGISTER;

    if (hasLink)
    {
        *getRegister(14) = *getRegister(15);
    }

    if (hasImmediate)
    {
        if ((addr & 0x00400000) != 0)
        {
            addr |= IMMEDIATE_23_SE;
        }
        *getRegister(15) = *getRegister(15) + (addr << 2);
    }
    else
    {
        *getRegister(15) = *getRegister(reg);
    }
}

void m20::Simulator::simulateSwi(int instr)
{
    throw SoftwareInterruptException();
}
