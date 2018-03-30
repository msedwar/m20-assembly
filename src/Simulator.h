/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 26, 2018
 * Description:
 *      Simulator for M20 Processor.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_SIMULATOR_H
#define M20_ASSEMBLY_SIMULATOR_H

#include <cassert>
#include <iostream>
#include <string>

namespace m20
{
    struct ProcessorException : public std::runtime_error
    {
        const int vector;

        ProcessorException(const std::string &message, const int vector)
            : std::runtime_error(message),
              vector(vector)
        {
            //
        }
    };

    struct UndefinedInstructionException : public ProcessorException
    {
        UndefinedInstructionException()
                : ProcessorException("Undefined Instruction", 4)
        {
            //
        }
    };

    struct SoftwareInterruptException : public ProcessorException
    {
        SoftwareInterruptException()
                : ProcessorException("Software Interrupt", 8)
        {
            //
        }
    };

    struct PrefetchAbortException : public ProcessorException
    {
        PrefetchAbortException()
                : ProcessorException("Prefetch Abort", 12)
        {
            //
        }
    };

    struct DataAbortException : public ProcessorException
    {
        DataAbortException()
                : ProcessorException("Data Abort", 16)
        {
            //
        }
    };

    struct InterruptException : public ProcessorException
    {
        InterruptException()
                : ProcessorException("Hardware interrupt", 20)
        {
            //
        }
    };

    struct UsageAbortException : public ProcessorException
    {
        UsageAbortException()
                : ProcessorException("Usage Abort", 24)
        {
            //
        }
    };

    class Simulator
    {
    public:
        Simulator(size_t memorySize)
                : MAX_ADDRESS(memorySize - 1),
                  mem(new char[memorySize]),
                  instructionsExecuted(0)
        {
            //
        }

        ~Simulator()
        {
            delete[] mem;
        }

        void load(const std::string &fname);

        void simulate();

        void printStatus();

    private:
        static const int MODE_USR = 0x00000000;
        static const int MODE_SVR = 0x00000001;
        static const int MODE_INT = 0x00000002;
        static const int MODE_ABT = 0x00000003;

        static const int ST_N = 0x80000000;
        static const int ST_Z = 0x40000000;
        static const int ST_C = 0x20000000;
        static const int ST_V = 0x10000000;

        const unsigned int MAX_ADDRESS;

        int reg_r[13];
        int reg_sp[4];
        int reg_lp[4];
        int reg_pc;
        int reg_st;
        int reg_sv[4];
        bool halt;

        char *mem;
        size_t instructionsExecuted;

        bool isCondition(int instr);
        void simulateData(int instr);
        void simulateLoad(int instr);
        void simulateBranch(int instr);
        void simulateSwi(int instr);

        inline int getMode()
        {
            if ((reg_st & MODE_ABT) == MODE_SVR)
            {
                return 1;
            }
            else if ((reg_st & MODE_ABT) == MODE_INT)
            {
                return 2;
            }
            else if ((reg_st & MODE_ABT) == MODE_ABT)
            {
                return 3;
            }
            // MODE_USR
            return 0;
        }

        int *getRegister(int reg)
        {
            if (reg >= 0 && reg <= 12)
            {
                return reg_r + reg;
            }
            else if (reg == 13)
            {
                return reg_sp + getMode();
            }
            else if (reg == 14)
            {
                return reg_lp + getMode();
            }
            else if (reg == 15)
            {
                return &reg_pc;
            }
            throw UsageAbortException();
        }

        int *getStatus(int reg)
        {
            if (reg == 0)
            {
                return &reg_st;
            }
            else if (reg == 1)
            {
                assert(getMode() != 0);
                return reg_sv + getMode();
            }
            else
            {
                assert(false);
                return nullptr;
            }
        }

        void storeWord(int addr, int val)
        {
            if (!(addr >= 0 && addr <= MAX_ADDRESS - 3))
            {
                throw DataAbortException();
            }
            storeByte(addr, (char) ((val >> 24) & 0xFF));
            storeByte(addr + 1, (char) ((val >> 16) & 0xFF));
            storeByte(addr + 2, (char) ((val >> 8) & 0xFF));
            storeByte(addr + 3, (char) (val & 0xFF));
        }

        void storeHalfword(int addr, int val)
        {
            if (!(addr >= 0 && addr <= MAX_ADDRESS - 1))
            {
                throw DataAbortException();
            }
            storeByte(addr, (char) ((val >> 8) & 0xFF));
            storeByte(addr + 1, (char) (val & 0xFF));
        }

        void storeByte(int addr, int val)
        {
            if (!(addr >= 0 && addr <= MAX_ADDRESS))
            {
                throw DataAbortException();
            }
            mem[addr] = (char) (val & 0xFF);
        }

        int loadWord(int addr)
        {
            if (!(addr >= 0 && addr <= MAX_ADDRESS - 3))
            {
                throw DataAbortException();
            }
            auto i0 = (unsigned int) mem[addr] & 0xFF;
            auto i1 = (unsigned int) mem[addr + 1] & 0xFF;
            auto i2 = (unsigned int) mem[addr + 2] & 0xFF;
            auto i3 = (unsigned int) mem[addr + 3] & 0xFF;
            return (int) ((unsigned) 0 | i0 << 24 | i1 << 16 | i2 << 8 | i3);
        }

        int loadHalfword(int addr)
        {
            if (!(addr >= 0 && addr <= MAX_ADDRESS - 1))
            {
                throw DataAbortException();
            }
            auto i0 = (unsigned int) mem[addr] & 0xFF;
            auto i1 = (unsigned int) mem[addr + 1] & 0xFF;
            return (int) ((unsigned) 0 | i0 << 8 | i1);
        }

        int loadByte(int addr)
        {
            if (!(addr >= 0 && addr <= MAX_ADDRESS))
            {
                throw DataAbortException();
            }
            auto i0 = (unsigned int) mem[addr] & 0xFF;
            return (int) ((unsigned) 0 | i0);
        }
    };
}

#endif // M20_ASSEMBLY_SIMULATOR_H
