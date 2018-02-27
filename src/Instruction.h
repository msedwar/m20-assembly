/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Instruction utilities and data structures
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_INSTRUCTION_H
#define M20_ASSEMBLY_INSTRUCTION_H

#include <string>

#include "Token.h"

namespace m20
{
    enum class InstructionCommand : char
    {
        NOOP,
        MUL,
        ADD,
        ADC,
        SUB,
        SBC,
        DIV,
        UDV,
        OR,
        AND,
        XOR,
        NOR,
        BIC,
        ROR,
        LSL,
        LSR,
        ASR,
        MOV,
        MVN,
        CMP,
        CMN,
        TST,
        TEQ,
        PUSH,
        POP,
        SRL,
        SRS,
        B,
        BWL,
        LDR,
        LDRH,
        LDRB,
        LDRSH,
        LDRSB,
        STR,
        STRH,
        STRB,
        SWI,
        HALT
    };

    enum class InstructionType : char
    {
        D3_REGISTER,
        D3_IMMEDIATE,
        D2_REGISTER,
        D2_IMMEDIATE,
        D2_LABEL,
        D1_REGISTER,
        D1_IMMEDIATE,
        EMPTY_TYPE,
        B_ABSOLUTE,
        B_RELATIVE,
        B_RELATIVE_LABEL,
        M_RELATIVE,
        M_RELATIVE_LABEL,
        M_DIRECT_INDEX,
        M_BASE_OFFSET,
        M_BASE_OFFSET_LABEL,
        M_BASE_INDEX,
        S_TYPE,

        DIRECTIVE_GLOBAL,
        DIRECTIVE_EXTERN,
        DIRECTIVE_ENTRY,
        DIRECTIVE_SECTION,
        DIRECTIVE_DATA,
        DIRECTIVE_DATA_ADDR,
        DIRECTIVE_SPACE,
        DIRECTIVE_LABEL
    };

    class Instruction
    {
    public:
        const Token &token;
        std::string label;
        InstructionType type;
        InstructionCommand command;
        Conditional cond = Conditional::AL;
        int rd = 0;
        int rn = 0;
        int operand = 0;
        char *data = nullptr;

        Instruction(const Token &token)
            : token(token)
        {
            //
        }
    };
}

#endif //M20_ASSEMBLY_INSTRUCTION_H
