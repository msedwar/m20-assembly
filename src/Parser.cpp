/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Syntax Parser for M20 Assembly. (Implementation)
 * =============================================================================
 */

#include <cassert>
#include <cctype>

#include "Parser.h"
#include "Assembler.h"

m20::M20Error::M20Error(const m20::M20ErrorType type,
                            const m20::Token &token,
                            const std::string &message)
    : type(type),
      token(token),
      message(message)
{
    //
}

m20::Parser::Parser(const std::vector<const m20::Token> &tokens)
    : index(0),
      tokens(tokens)
{
    //
}

bool m20::Parser::parse()
{
    if (tokens.empty())
    {
        return true;
    }

    try
    {
        while (hasNext())
        {
            const Token &current = next();
            try
            {
                parseStatement(current);
            }
            catch (const int e)
            {
                //
            }
        }
    }
    catch (const std::out_of_range &e)
    {
        errors.emplace_back(M20ErrorType::SYNTAX, getCurrent(),
                            "Unexpected EOF");
    }

    return errors.empty();
}

void m20::Parser::printErrors()
{
    int totalErrors = 0;
    int line = 0;
    for (const auto &e : errors)
    {
        if (e.token.line == line)
        {
            continue;
        }
        line = e.token.line;
        ++totalErrors;
        std::cout << "Syntax Error (" << e.token.line
                  << ":" << e.token.column
                  << "): " << e.message << "\n";
        std::cout << "\t" << e.token.raw << "\n";
        std::cout << "\t^ " << getTokenTypeName(e.token.type) << std::endl;
    }

    std::cout << totalErrors << " error(s) found." << std::endl;
}

void m20::Parser::parseStatement(const Token &current)
{
    switch (current.type)
    {
        case TokenType::KEYWORD:
        {
            if (streqi(current.raw, "global")
                || streqi(current.raw, "extern")
                || streqi(current.raw, "entry"))
            {
                Instruction d(current);
                const Token &label = accept(TokenType::LABEL);
                d.label = label.raw;
                if (streqi(current.raw, "global"))
                {
                    d.type = InstructionType::DIRECTIVE_GLOBAL;
                }
                else if (streqi(current.raw, "extern"))
                {
                    d.type = InstructionType::DIRECTIVE_EXTERN;
                }
                else // Entry
                {
                    d.type = InstructionType::DIRECTIVE_ENTRY;
                }
                items.push_back(d);
            }
            else if (streqi(current.raw, "section"))
            {
                Instruction d(current);
                const Token &label = accept(TokenType::KEYWORD);
                if (streqi(label.raw, ".text")
                    || streqi(label.raw, ".data"))
                {
                    d.type = InstructionType::DIRECTIVE_SECTION;
                    d.label = label.raw;
                    items.push_back(d);
                }
                else
                {
                    errors.emplace_back(M20ErrorType::SYNTAX, current,
                                        "Unexpected symbol, expected '.text' "
                                        "or '.data'");
                }
            }
            else if (streqi(current.raw, "space"))
            {
                Instruction d(current);
                const Token &num = accept(TokenType::NUMBER);
                d.type = InstructionType::DIRECTIVE_SPACE;
                d.operand = parseArgNumber(num);
                items.push_back(d);
            }
            else if (streqi(current.raw, "db")
                     || streqi(current.raw, "dh")
                     || streqi(current.raw, "dw")
                     || streqi(current.raw, "dd"))
            {
                Instruction d(current);
                d.type = InstructionType::DIRECTIVE_DATA;
                std::vector<char> bytes;
                int maxBytes = 0;

                if (streqi(current.raw, "db"))
                {
                    if (peek().type == TokenType::STRING)
                    {
                        const Token &str = next();
                        parseDataString(str.raw.substr(1, str.raw.length() - 2),
                                        bytes);
                    }
                    else
                    {
                        maxBytes = 1;
                    }
                }
                else if (streqi(current.raw, "dh"))
                {
                    maxBytes = 2;
                }
                else if (streqi(current.raw, "dw"))
                {
                    if (peek().type == TokenType::LABEL
                            || (peek().type == TokenType::KEYWORD
                                && peek().raw == "$"))
                    {
                        const Token &label = next();
                        d.label = label.raw;
                        d.type = InstructionType::DIRECTIVE_DATA_ADDR;
                    }
                    else
                    {
                        maxBytes = 4;
                    }
                }
                else // Double
                {
                    maxBytes = 8;
                }

                if (maxBytes > 0)
                {
                    const Token &num = next();
                    parseDataNumber(num, maxBytes, bytes);

                    while (hasNext() && peek().type == TokenType::COMMA)
                    {
                        expect(TokenType::COMMA);
                        const Token &num2 = accept(TokenType::NUMBER);
                        parseDataNumber(num2, maxBytes, bytes);
                    }
                }

                d.operand = (unsigned int) bytes.size();
                d.data = new char[d.operand];
                for (size_t i = 0; i < bytes.size(); ++i)
                {
                    d.data[i] = bytes[i];
                }
                items.push_back(d);
            }
            else
            {
                errors.emplace_back(M20ErrorType::SYNTAX, current,
                                    "Unknown directive");
            }
            break;
        }
        case TokenType::LABEL:
        {
            Instruction d(current);
            if (peek().type == TokenType::DECLARE)
            {
                expect(TokenType::DECLARE);
            }
            else
            {
                errors.emplace_back(M20ErrorType::SYNTAX, current,
                                    "Unknown symbol, did you mean a command?");
                break;
            }
            d.type = InstructionType::DIRECTIVE_LABEL;
            d.label = current.raw;
            items.push_back(d);
            break;
        }
        case TokenType::NUMBER:
        {
            Instruction d(current);
            std::vector<char> bytes;
            parseDataNumber(current, 4, bytes);
            d.type = InstructionType::DIRECTIVE_DATA;

            while (hasNext() && peek().type == TokenType::COMMA)
            {
                expect(TokenType::COMMA);
                const Token &num = accept(TokenType::NUMBER);
                parseDataNumber(num, 4, bytes);
            }

            d.operand = (unsigned int) bytes.size();
            d.data = new char[d.operand];
            for (size_t i = 0; i < bytes.size(); ++i)
            {
                d.data[i] = bytes[i];
            }
            items.push_back(d);
            break;
        }
        case TokenType::D3_INSTR:
        {
            items.push_back(get3Arg(current));
            break;
        }
        case TokenType::D2_INSTR:
        {
            items.push_back(get2Arg(current));
            break;
        }
        case TokenType::D1_INSTR:
        {
            items.push_back(get1Arg(current));
            break;
        }
        case TokenType::EMPTY_INSTR:
        {
            Instruction instr(current);
            instr.cond = current.condition;
            if (streqi(current.raw, "noop"))
            {
                instr.type = InstructionType::EMPTY_TYPE;
                instr.command = InstructionCommand::NOOP;
            }
            else if (streqi(current.raw, "halt"))
            {
                instr.type = InstructionType::EMPTY_TYPE;
                instr.command = InstructionCommand::HALT;
            }
            else
            {
                assert(false);
            }
            items.push_back(instr);
            break;
        }
        case TokenType::INSTRUCTION:
        {
            Instruction instr(current);
            instr.cond = current.condition;
            if (streqi(current.raw, "swi"))
            {
                instr.type = InstructionType::S_TYPE;
                instr.command = InstructionCommand::SWI;
                const Token &tok = accept(TokenType::NUMBER);
                instr.operand = parseInt(tok, 24);
            }
            else if (streqi(current.raw, "b")
                    || streqi(current.raw, "bwl"))
            {
                if (streqi(current.raw, "b"))
                {
                    instr.command = InstructionCommand::B;
                }
                else if (streqi(current.raw, "bwl"))
                {
                    instr.command = InstructionCommand::BWL;
                }
                else
                {
                    assert(false);
                }

                const Token &tok = next();
                if (tok.type == TokenType::NUMBER)
                {
                    instr.type = InstructionType::B_ABSOLUTE;
                    instr.operand = parseInt(tok, 16);
                }
                else if (tok.type == TokenType::LABEL)
                {
                    instr.type = InstructionType::B_RELATIVE_LABEL;
                    instr.label = tok.raw;
                }
                else if (tok.type == TokenType::REGISTER)
                {
                    instr.type = InstructionType::B_RELATIVE;
                    instr.operand = parseRegister(tok.raw);
                }
                else
                {
                    errors.emplace_back(M20ErrorType::SYNTAX, tok,
                                        "Expected numeric literal or label");
                    break;
                }
            }
            else
            {
                assert(false);
            }
            items.push_back(instr);
            break;
        }
        case TokenType::MEM_INSTR:
        {
            items.push_back(getMem(current));
            break;
        }
        default:
        {
            errors.emplace_back(M20ErrorType::SYNTAX, current,
                                "Invalid syntax");
            break;
        }
    }
}

const m20::Token &m20::Parser::getCurrent()
{
    return tokens.at(index - 1);
}

const m20::Token &m20::Parser::next()
{
    return tokens.at(index++);
}

const m20::Token &m20::Parser::peek()
{
    return tokens.at(index);
}

bool m20::Parser::hasNext()
{
    return index < tokens.size();
}

const m20::Token &m20::Parser::accept(const m20::TokenType type)
{
    const Token &tok = next();
    if (tok.type == type)
    {
        return tok;
    }
    errors.emplace_back(M20ErrorType::SYNTAX, tok,
                        "Unexpected symbol, expected "
                        + getTokenTypeName(type));
    throw 0;
}

void m20::Parser::expect(const m20::TokenType type)
{
    const Token &tok = next();
    if (tok.type != type)
    {
        errors.emplace_back(M20ErrorType::SYNTAX, tok,
                            "Unexpected symbol, expected "
                            + getTokenTypeName(type));
        throw 0;
    }
}

size_t m20::Parser::parseNumber(const Token &num, int maxBits)
{
    assert(num.raw.size() >= 2);

    size_t result = 0;
    char negative = 0;
    try
    {
        if (tolower(num.raw[0]) == '#')  // Decimal
        {
            if (num.raw[1] == '-')
            {
                negative = 1;
            }
            else
            {
                negative = 2;
            }
            result = (unsigned) std::stol(num.raw.substr(1), nullptr, 10);
        }
        else if (tolower(num.raw[1]) == 'x')
        {
            result = std::stoul(num.raw.substr(2), nullptr, 16);
        }
        else if (tolower(num.raw[1]) == 'b')
        {
            result = std::stoul(num.raw.substr(2), nullptr, 2);
        }
        else
        {
            result = std::stoul(num.raw.substr(1), nullptr, 8);
        }
    }
    catch (const std::invalid_argument &e)
    {
        errors.emplace_back(M20ErrorType::SYNTAX, num,
                         "Invalid numeric literal");
        throw 0;
    }

    size_t mask = ((size_t) 0xFFFFFFFFFFFFFFFF) << maxBits;
    if (maxBits == 64)
    {
        mask = 0;
    }
    result = result & ~mask;

    size_t masked = result & (((size_t) 0x1) << (maxBits - 1));
    if ((masked && negative == 2)
        || (!masked && negative == 1))
    {
        errors.emplace_back(M20ErrorType::SYNTAX, num,
                            "Invalid numeric literal "
                                    "(signed overflow)");
        throw 0;
    }

    mask = ((size_t) 0xFFFFFFFFFFFFFFFF) << maxBits;
    if (maxBits == 64)
    {
        mask = 0;
    }

    if (result & mask)
    {
        errors.emplace_back(M20ErrorType::SYNTAX, num,
                            "Numeric literal out of range (should be "
                            + std::to_string(maxBits) + " bits)");
        throw 0;
    }

    return result;
}

unsigned int m20::Parser::parseArgNumber(const Token &num)
{
    return (unsigned int) (0xFFFFFFFF & parseNumber(num, 32));
}

unsigned int m20::Parser::parseInt(const Token &num, int maxBits)
{
    return (unsigned int) (0xFFFFFFFF & parseNumber(num, maxBits));
}

void m20::Parser::parseDataNumber(const Token &num,
                                  int maxBytes,
                                  std::vector<char> &bytes)
{
    size_t x = parseNumber(num, maxBytes * 8);
    for (int i = 1; i <= maxBytes; ++i)
    {
        bytes.push_back((char) ((x >> ((maxBytes - i) * 8)) & 0xFF));
    }
}

void m20::Parser::parseDataString(const std::string &str,
                                  std::vector<char> &bytes)
{
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '\\')
        {
            assert(i + 1 < str.length());
            ++i;
            switch (str[i])
            {
                case '\'':
                    bytes.push_back('\'');
                    break;
                case '\"':
                    bytes.push_back('\"');
                    break;
                case '\\':
                    bytes.push_back('\\');
                    break;
                case 'n':
                    bytes.push_back('\n');
                    break;
                case 'r':
                    bytes.push_back('\r');
                    break;
                case 't':
                    bytes.push_back('\t');
                    break;
                case '0':
                    bytes.push_back('\0');
                    break;
                case 'x':
                    assert(false);
                    break;
                default:
                    errors.emplace_back(M20ErrorType::SYNTAX, getCurrent(),
                                        "Invalid escape code");
                    throw 0;
            }
        }
        else
        {
            bytes.push_back(str[i]);
        }
    }
}

unsigned int m20::Parser::parseRegister(const std::string &str,
                                        bool status)
{
    if (status)
    {
        if (str == "st")
        {
            return 0;
        }
        else if (str == "sv")
        {
            return 1;
        }
        else
        {
            errors.emplace_back(M20ErrorType::SYNTAX, getCurrent(),
                                "Invalid status register");
            throw 0;
        }
    }
    else
    {
        if (str == "pc")
        {
            return 15;
        }
        else if (str == "lp")
        {
            return 14;
        }
        else if (str == "sp")
        {
            return 13;
        }
    }

    try
    {
        unsigned int reg = (unsigned) stol(str.substr(1), nullptr, 10);
        if (reg > 12)
        {
            throw std::out_of_range("");
        }
        return reg;
    }
    catch (const std::out_of_range &e)
    {
        errors.emplace_back(M20ErrorType::SYNTAX, getCurrent(),
                            "Invalid register");
        throw 0;
    }
}

m20::Instruction m20::Parser::get3Arg(const m20::Token &command)
{
    Instruction instr(command);
    instr.cond = command.condition;

    if (streqi(command.raw, "mul"))
    {
        instr.command = InstructionCommand::MUL;
    }
    else if (streqi(command.raw, "add"))
    {
        instr.command = InstructionCommand::ADD;
    }
    else if (streqi(command.raw, "adc"))
    {
        instr.command = InstructionCommand::ADC;
    }
    else if (streqi(command.raw, "sub"))
    {
        instr.command = InstructionCommand::SUB;
    }
    else if (streqi(command.raw, "sbc"))
    {
        instr.command = InstructionCommand::SBC;
    }
    else if (streqi(command.raw, "div"))
    {
        instr.command = InstructionCommand::DIV;
    }
    else if (streqi(command.raw, "udv"))
    {
        instr.command = InstructionCommand::UDV;
    }
    else if (streqi(command.raw, "or"))
    {
        instr.command = InstructionCommand::OR;
    }
    else if (streqi(command.raw, "and"))
    {
        instr.command = InstructionCommand::AND;
    }
    else if (streqi(command.raw, "xor"))
    {
        instr.command = InstructionCommand::XOR;
    }
    else if (streqi(command.raw, "nor"))
    {
        instr.command = InstructionCommand::NOR;
    }
    else if (streqi(command.raw, "bic"))
    {
        instr.command = InstructionCommand::BIC;
    }
    else if (streqi(command.raw, "ror"))
    {
        instr.command = InstructionCommand::ROR;
    }
    else if (streqi(command.raw, "lsl"))
    {
        instr.command = InstructionCommand::LSL;
    }
    else if (streqi(command.raw, "lsr"))
    {
        instr.command = InstructionCommand::LSR;
    }
    else if (streqi(command.raw, "asr"))
    {
        instr.command = InstructionCommand::ASR;
    }
    else
    {
        assert(false);
    }

    const Token &tok1 = accept(TokenType::REGISTER);
    instr.rd = parseRegister(tok1.raw);

    expect(TokenType::COMMA);

    const Token &tok2 = accept(TokenType::REGISTER);
    instr.rn = parseRegister(tok2.raw);

    expect(TokenType::COMMA);

    const Token &tok3 = next();
    if (tok3.type == TokenType::NUMBER)
    {
        instr.operand = parseInt(tok3, 12);
        instr.type = InstructionType::D3_IMMEDIATE;
    }
    else if (tok3.type == TokenType::REGISTER)
    {
        instr.operand = parseRegister(tok3.raw);
        instr.type = InstructionType::D3_REGISTER;
    }
    else
    {
        errors.emplace_back(M20ErrorType::SYNTAX, tok3,
                            "Expected register or immediate");
        throw 0;
    }
    return instr;
}

m20::Instruction m20::Parser::get2Arg(const m20::Token &command)
{
    Instruction instr(command);
    instr.cond = command.condition;

    bool allowedLiteral = true;
    bool allowedLabel = false;
    bool statusFirst = false;
    bool statusSecond = false;
    if (streqi(command.raw, "mov"))
    {
        instr.command = InstructionCommand::MOV;
        allowedLabel = true;
    }
    else if (streqi(command.raw, "mvn"))
    {
        instr.command = InstructionCommand::MVN;
        allowedLabel = true;
    }
    else if (streqi(command.raw, "cmp"))
    {
        instr.command = InstructionCommand::CMP;
    }
    else if (streqi(command.raw, "cmn"))
    {
        instr.command = InstructionCommand::CMN;
    }
    else if (streqi(command.raw, "tst"))
    {
        instr.command = InstructionCommand::TST;
    }
    else if (streqi(command.raw, "teq"))
    {
        instr.command = InstructionCommand::TEQ;
    }
    else if (streqi(command.raw, "srl"))
    {
        instr.command = InstructionCommand::SRL;
        allowedLiteral = false;
        statusSecond = true;
    }
    else if (streqi(command.raw, "srs"))
    {
        instr.command = InstructionCommand::SRS;
        statusFirst = true;
    }
    else
    {
        assert(false);
    }

    const Token &tok1 = accept(TokenType::REGISTER);
    instr.rd = parseRegister(tok1.raw, statusFirst);

    expect(TokenType::COMMA);

    const Token &tok2 = next();
    if (allowedLiteral && tok2.type == TokenType::NUMBER)
    {
        instr.operand = parseInt(tok2, 16);
        instr.type = InstructionType::D2_IMMEDIATE;
    }
    else if (tok2.type == TokenType::REGISTER)
    {
        instr.operand = parseRegister(tok2.raw, statusSecond);
        instr.type = InstructionType::D2_REGISTER;
    }
    else if (allowedLabel && tok2.type == TokenType::LABEL)
    {
        instr.label = tok2.raw;
        instr.type = InstructionType::D2_LABEL;
    }
    else
    {
        errors.emplace_back(M20ErrorType::SYNTAX, tok2,
                            !allowedLiteral
                            ? "Expected register"
                            : (!allowedLabel
                            ? "Expected register or immediate"
                            : "Expected register, immediate, or label"));
        throw 0;
    }
    return instr;
}

m20::Instruction m20::Parser::get1Arg(const m20::Token &command)
{
    Instruction instr(command);
    instr.cond = command.condition;

    bool allowedLiteral = true;
    if (streqi(command.raw, "push"))
    {
        instr.command = InstructionCommand::PUSH;
    }
    else if (streqi(command.raw, "pop"))
    {
        instr.command = InstructionCommand::POP;
        allowedLiteral = false;
    }
    else
    {
        assert(false);
    }

    const Token &tok = next();
    if (allowedLiteral && tok.type == TokenType::NUMBER)
    {
        instr.operand = parseInt(tok, 20);
        instr.type = InstructionType::D1_IMMEDIATE;
    }
    else if (tok.type == TokenType::REGISTER)
    {
        instr.operand = parseRegister(tok.raw);
        instr.type = InstructionType ::D1_REGISTER;
    }
    else
    {
        errors.emplace_back(M20ErrorType::SYNTAX, tok,
                            !allowedLiteral
                            ? "Expected register"
                            : "Expected register or immediate");
        throw 0;
    }
    return instr;
}

m20::Instruction m20::Parser::getMem(const m20::Token &command)
{
    Instruction instr(command);
    instr.cond = command.condition;

    if (streqi(command.raw, "ldr"))
    {
        instr.command = InstructionCommand::LDR;
    }
    else if (streqi(command.raw, "ldrh"))
    {
        instr.command = InstructionCommand::LDRH;
    }
    else if (streqi(command.raw, "ldrb"))
    {
        instr.command = InstructionCommand::LDRB;
    }
    else if (streqi(command.raw, "ldrsh"))
    {
        instr.command = InstructionCommand::LDRSH;
    }
    else if (streqi(command.raw, "ldrsb"))
    {
        instr.command = InstructionCommand::LDRSB;
    }
    else if (streqi(command.raw, "str"))
    {
        instr.command = InstructionCommand::STR;
    }
    else if (streqi(command.raw, "strh"))
    {
        instr.command = InstructionCommand::STRH;
    }
    else if (streqi(command.raw, "strb"))
    {
        instr.command = InstructionCommand::STRB;
    }
    else
    {
        assert(false);
    }

    const Token &tok1 = accept(TokenType::REGISTER);
    instr.rd = parseRegister(tok1.raw);

    expect(TokenType::COMMA);

    const Token &tok2 = next();
    if (tok2.type == TokenType::REGISTER)
    {
        instr.rn = parseRegister(tok2.raw);

        if (peek().type == TokenType::COMMA)
        {
            expect(TokenType::COMMA);

            const Token &tok3 = next();
            if (tok3.type == TokenType::REGISTER)
            {
                instr.operand = parseRegister(tok3.raw);
                instr.type = InstructionType::M_BASE_INDEX;
            }
            else if (tok3.type == TokenType::NUMBER)
            {
                instr.operand = parseInt(tok3, 12);
                instr.type = InstructionType::M_BASE_OFFSET;
            }
            else if (tok3.type == TokenType::LABEL)
            {
                instr.label = tok3.raw;
                instr.type = InstructionType::M_BASE_OFFSET_LABEL;
            }
            else
            {
                errors.emplace_back(M20ErrorType::SYNTAX, tok3,
                                    "Expected register, immediate, or label");
                throw 0;
            }
        }
        else
        {
            instr.operand = instr.rn;
            instr.rn = 0;
            instr.type = InstructionType::M_DIRECT_INDEX;
        }
    }
    else if (tok2.type == TokenType::NUMBER)
    {
        instr.operand = parseInt(tok2, 16);
        instr.type = InstructionType::M_RELATIVE;
    }
    else if (tok2.type == TokenType::LABEL)
    {
        instr.label = tok2.raw;
        instr.type = InstructionType::M_RELATIVE_LABEL;
    }
    else
    {
        errors.emplace_back(M20ErrorType::SYNTAX, tok2,
                            "Expected register, immediate, or label");
        throw 0;
    }

    return instr;
}

bool m20::Parser::streqi(const std::string &l, const std::string &r)
{
    if (l.size() != r.size())
    {
        return false;
    }

    for (int i = 0; i < l.size(); ++i)
    {
        if (tolower(l[i]) != tolower(r[i]))
        {
            return false;
        }
    }

    return true;
}
