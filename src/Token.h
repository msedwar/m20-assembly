/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Definitions of Token and TokenType used when assembling M20.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_TOKEN_H
#define M20_ASSEMBLY_TOKEN_H

#include <iostream>
#include <map>
#include <regex>
#include <string>

namespace m20
{
    /**
     * Enumeration of types of tokens in M20 assembly
     */
    enum class TokenType : unsigned int
    {
        INVALID     = 0,
        WHITESPACE  = 1,
        COMMENT     = 2,
        NUMBER      = 3,
        KEYWORD     = 4,
        REGISTER    = 5,
        COMMA       = 6,
        DECLARE     = 7,
        STRING      = 8,
        D3_INSTR    = 9,
        D2_INSTR    = 10,
        D1_INSTR    = 11,
        EMPTY_INSTR = 12,
        MEM_INSTR   = 13,
        INSTRUCTION = 14,
        LABEL       = 15
    };

    enum class Conditional : unsigned int
    {
        EQ,
        NE,
        CS,
        CC,
        MI,
        PL,
        VS,
        VC,
        HI,
        LS,
        GE,
        LT,
        GT,
        LE,
        AL
    };

    /**
     * Data structure used to store data about a token
     */
    struct Token
    {
        std::string raw;
        TokenType type;
        Conditional condition;
        int line;
        int column;
        bool updateStatus;

        Token();

        friend std::ostream &operator<<(std::ostream &os, const Token &token);
    };

    const unsigned int TOKEN_COUNT = 16;

    const std::string TOKEN_NAMES[] = {
            "INVALID",
            "WHITESPACE",
            "COMMENT",
            "NUMBER",
            "KEYWORD",
            "REGISTER",
            "COMMA",
            "DECLARE",
            "STRING",
            "D3 INSTRUCTION",
            "D2 INSTRUCTION",
            "D1 INSTRUCTION",
            "EMPTY INSTRUCTION",
            "MEM INSTRUCTION",
            "INSTRUCTION",
            "LABEL"
    };

    const std::regex TOKEN_REGEX[] = {
            // INVALID
            std::regex("[^ \n\t\r]+"),
            // WHITESPACE
            std::regex("[ \n\t\r]"),
            // COMMENT
            std::regex(";[^\n]*"),
            // NUMBER
            std::regex("(#(0|\\-?([1-9][0-9]*))|"
                       "(0x[0-9a-fA-F]{1,16})|"
                       "(01?[0-7]{1,21})|"
                       "(0b[0-1]{1,64}))"),
            // KEYWORD
            std::regex("(global|extern|entry|section"
                       "|\\.text|\\.data"
                       "|d[bhwd]|space|\\$)", std::regex_constants::icase),
            // REGISTER
            std::regex("(r10|r11|r12|r0|r1|r2|r3|r4|r5|r6|r7|r8|r9"
                       "|sp|lp|pc|st|sv)", std::regex_constants::icase),
            // COMMA
            std::regex(","),
            // DECLARE
            std::regex(":"),
            // STRING
            std::regex("\"(\\\\[\'\"\\\\nrt0]|\\\\x[0-9a-fA-F]{2}|[^\"])*\""),
            // D3 INSTRUCTION
            std::regex("(mul|add|adc|sub|sbc|div|udv|"
                       "or|and|xor|nor|bic|ror|lsl|lsr|asr)"
                       "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?(\\.s)?",
                       std::regex_constants::icase),
            // D2 INSTRUCTION
            std::regex("(mov|mvn|cmp|cmn|tst|teq|srs|srl)"
                       "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?",
                       std::regex_constants::icase),
            // D1 INSTRUCTION
            std::regex("(push|pop)"
                       "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?",
                       std::regex_constants::icase),
            // EMPTY INSTRUCTION
            std::regex("(noop|halt)"
                       "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?",
                       std::regex_constants::icase),
            // MEM INSTRUCTION
            std::regex("(ldrsb|ldrsh|ldrb|ldrh|ldr|strb|strh|str)"
                       "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?",
                       std::regex_constants::icase),
            // INSTRUCTION
            std::regex("(b(wl)?|swi)"
                       "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?",
                       std::regex_constants::icase),
            // LABEL
            std::regex("[A-Za-z_][A-Za-z0-9_]*")
    };

    const std::regex CONDITIONAL_REGEX(
            "(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)$",
            std::regex_constants::icase);
    const std::regex STATUS_UPDATE_REGEX(
            "(\\.s)$",
            std::regex_constants::icase);

    const std::string &getTokenTypeName(TokenType type);
}

#endif // M20_ASSEMBLY_TOKEN_H
