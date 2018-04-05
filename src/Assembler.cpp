/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Assembler for M20 Assembly. (Implementation)
 * =============================================================================
 */

#include <cmath>
#include <iomanip>

#include "Assembler.h"

const m20::Token m20::Assembler::BEGIN_TOKEN;

bool m20::Assembler::assemble(const std::string &inputFileName,
                              const std::string &outputFileName)
{
    std::ifstream inputFile(inputFileName);

    assert(inputFile.is_open());

    std::vector<std::string> lines;
    std::stringstream input;

    std::string line;
    while (std::getline(inputFile, line))
    {
        lines.push_back(line);
        input << line << "\n";
    }

    inputFile.close();

    std::vector<const Token> tokens;
    Lexer::lex(input.str(), tokens);

    Parser parser(tokens);
    parser.parse();

    if (parser.getNumErrors() > 0)
    {
        printErrors(inputFileName, parser.getErrors(), lines);
        return false;
    }

    bytes.reserve(parser.getItems().size());
    generate(parser.getItems());

    for (const auto &global : globals)
    {
        if (labels.find(global.label) == labels.end())
        {
            errors.emplace_back(M20ErrorType::DIRECTIVE, global.token,
                                "Global has no corresponding local label");
        }
    }

    if (!errors.empty())
    {
        printErrors(inputFileName, errors, lines);
        return false;
    }

    try
    {
        updateLabels();
    }
    catch (const std::overflow_error &e)
    {
        errors.emplace_back(M20ErrorType::LABEL, BEGIN_TOKEN,
                            "Too many labels were created in this file");
    }

    if (!errors.empty())
    {
        printErrors(inputFileName, errors, lines);
        return false;
    }

    std::ofstream outputFile(outputFileName, std::ios::out | std::ios::binary);
    assert(outputFile.is_open());

    char buffer[4];
    outputFile.write(intToBytes(0x7F4D3230, buffer), 4);   // Magic Bytes
    outputFile.write(intToBytes(0x00000001, buffer), 4);   // Version

    // Section size
    outputFile.write(intToBytes((int) sections.size(), buffer), 4);
    // Code section size
    outputFile.write(intToBytes((int) bytes.size(), buffer), 4);
    // String table size
    outputFile.write(intToBytes((int) stringBytes.size(), buffer), 4);
    // Symbol table size
    outputFile.write(intToBytes((int) symbolTable.size(), buffer), 4);
    // Relocation table size
    outputFile.write(intToBytes((int) relocationTable.size(), buffer), 4);

    for (const auto &s : sections)
    {
        char text = s.text ? (char) 0xFF : (char) 0x00;
        outputFile.write(&text, 1);
        outputFile.write(intToBytes(s.end, buffer), 4);
    }

    outputFile.write(bytes.data(), bytes.size());
    outputFile.write(stringBytes.data(), stringBytes.size());
    outputFile.write(symbolTable.data(), symbolTable.size());
    outputFile.write(relocationTable.data(), relocationTable.size());

    outputFile.close();

//    printByteDump(4, true);

    return true;
}

void m20::Assembler::generate(std::vector<m20::Instruction> &items)
{
    if (items.empty())
    {
        errors.emplace_back(M20ErrorType::DIRECTIVE, BEGIN_TOKEN,
                            "No sections defined");
        return;
    }
    for (const m20::Instruction &instr : items)
    {
        if (instr.type == InstructionType::DIRECTIVE_GLOBAL)
        {
            if (std::find(globals.begin(), globals.end(), instr.label)
                == globals.end())
            {
                globals.emplace_back(instr.label, instr.token);
            }
            else
            {
                errors.emplace_back(M20ErrorType::DIRECTIVE, instr.token,
                                    "Duplicate global label definition");
            }
        }
        else if (instr.type == InstructionType::DIRECTIVE_EXTERN)
        {
            if (std::find(externs.begin(), externs.end(), instr.label)
                == externs.end())
            {
                externs.emplace(instr.label, instr.token);
            }
            else
            {
                errors.emplace_back(M20ErrorType::DIRECTIVE, instr.token,
                                    "Duplicate extern label definition");
            }
        }
        else if (instr.type == InstructionType::DIRECTIVE_ENTRY)
        {
            if (!hasEntrypoint)
            {
                if (std::find(globals.begin(), globals.end(), instr.label)
                    == globals.end())
                {
                    hasEntrypoint = true;
                    entryPoint = instr.label;

                    globals.emplace_back(instr.label, instr.token);
                }
                else
                {
                    errors.emplace_back(M20ErrorType::DIRECTIVE, instr.token,
                                        "Duplicate entry label and global "
                                        "label definition");
                }
            }
            else
            {
                errors.emplace_back(M20ErrorType::DIRECTIVE, instr.token,
                                    "Duplicate entry label definition");
            }
        }
        else if (instr.type == InstructionType::DIRECTIVE_SECTION)
        {
            if (!sections.empty())
            {
                sections.back().end = (unsigned int) bytes.size();
            }
            sections.emplace_back(instr.token, (unsigned int) bytes.size(),
                                  instr.label == ".text");
        }
        else if (instr.type == InstructionType::DIRECTIVE_DATA)
        {
            if (sections.empty())
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    items.front().token,
                                    "Data declarations must be in a section");
            }
            else if (sections.back().text && instr.operand % 4 != 0)
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    instr.token,
                                    "Non-aligned data declarations must be in "
                                            "a non-text section");
            }
            else
            {
                for (int i = 0; i < instr.operand; ++i)
                {
                    bytes.push_back(instr.data[i]);
                }
                delete[] instr.data;
            }
        }
        else if (instr.type == InstructionType::DIRECTIVE_DATA_ADDR)
        {
            if (sections.empty())
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    items.front().token,
                                    "Data declarations must be in a section");
            }
            else
            {
                if (instr.label == "$")
                {
                    fixups.emplace_back("$",
                                        instr.token,
                                        (unsigned int) bytes.size(),
                                        instr.type);
                }
                else
                {
                    fixups.emplace_back(instr.label,
                                        instr.token,
                                        (unsigned int) bytes.size(),
                                        instr.type);
                }

                bytes.push_back(0);
                bytes.push_back(0);
                bytes.push_back(0);
                bytes.push_back(0);
            }
        }
        else if (instr.type == InstructionType::DIRECTIVE_SPACE)
        {
            if (sections.empty() || sections.back().text)
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    instr.token,
                                    "Data declarations must be in a "
                                    "non-text section");
            }
            else
            {
                for (int i = 0; i < instr.operand; ++i)
                {
                    bytes.push_back((char) 0x00);
                }
            }
        }
        else if (instr.type == InstructionType::DIRECTIVE_LABEL)
        {
            if (labels.find(instr.label) == labels.end())
            {
                Label label = {
                        instr.label,
                        (unsigned int) bytes.size(),
                        (unsigned int) sections.size() - 1
                };
                labels[instr.label] = label;
                labelLookup[(unsigned int) bytes.size()] = label;
            }
            else
            {
                errors.emplace_back(M20ErrorType::DIRECTIVE, instr.token,
                                    "Duplicate label definition");
            }
        }
        else if (instr.type == InstructionType::D3_REGISTER
            || instr.type == InstructionType::D3_IMMEDIATE
            || instr.type == InstructionType::D2_REGISTER
            || instr.type == InstructionType::D2_IMMEDIATE
            || instr.type == InstructionType::D2_LABEL
            || instr.type == InstructionType::D1_REGISTER
            || instr.type == InstructionType::D1_IMMEDIATE
            || instr.type == InstructionType::EMPTY_TYPE)
        {
            if (sections.empty())
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    items.front().token,
                                    "Instructions must be in a section");
            }
            else
            {
                generateDataProcessing(instr);
            }
        }
        else if (instr.type == InstructionType::M_RELATIVE
                 || instr.type == InstructionType::M_RELATIVE_LABEL
                 || instr.type == InstructionType::M_DIRECT_INDEX
                 || instr.type == InstructionType::M_BASE_OFFSET
                 || instr.type == InstructionType::M_BASE_OFFSET_LABEL
                 || instr.type == InstructionType::M_BASE_INDEX)
        {
            if (sections.empty())
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    items.front().token,
                                    "Instructions must be in a section");
            }
            else
            {
                generateDataLoad(instr);
            }
        }
        else if (instr.type == InstructionType::B_ABSOLUTE
                 || instr.type == InstructionType::B_RELATIVE
                 || instr.type == InstructionType::B_RELATIVE_LABEL)
        {
            if (sections.empty())
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    items.front().token,
                                    "Instructions must be in a section");
            }
            else
            {
                generateBranch(instr);
            }
        }
        else if (instr.type == InstructionType::S_TYPE)
        {
            if (sections.empty())
            {
                errors.emplace_back(M20ErrorType::SECTION,
                                    items.front().token,
                                    "Instructions must be in a section");
            }
            else
            {
                generateSwi(instr);
            }
        }
        else
        {
            assert(false);
        }
    }
    if (sections.empty())
    {
        errors.emplace_back(M20ErrorType::DIRECTIVE, items.front().token,
                            "No sections defined");
    }
    else
    {
        sections.back().end = (unsigned int) bytes.size();
    }
}

void m20::Assembler::generateDataProcessing(const Instruction &instr)
{
    int code = 0;
    code |= getConditionNibble(instr.cond);
    code |= getDataOpcode(instr.command);

    int hasUpdate = 0x04000000;
    if (!instr.token.updateStatus)
    {
        hasUpdate = 0;
    }
    code |= hasUpdate;

    int hasImmediate = 0x02000000;
    if (instr.type == InstructionType::D3_REGISTER
        || instr.type == InstructionType::D2_REGISTER
        || instr.type == InstructionType::D1_REGISTER
        || instr.type == InstructionType::EMPTY_TYPE)
    {
        hasImmediate = 0;
    }
    code |= hasImmediate;

    int operandMask = 0x00000FFF;

    if (instr.type == InstructionType::D2_REGISTER
        || instr.type == InstructionType::D2_IMMEDIATE
        || instr.type == InstructionType::D2_LABEL)
    {
        operandMask = 0x0000FFFF;
    }
    else if (instr.type == InstructionType::D1_REGISTER
            || instr.type == InstructionType::D1_IMMEDIATE)
    {
        operandMask = 0x000FFFFF;
    }
    else if (instr.type == InstructionType::EMPTY_TYPE)
    {
        operandMask = 0x00000000;
    }

    code |= (instr.rd << 16) & 0x000F0000;
    code |= (instr.rn << 12) & 0x0000F000;
    code |= instr.operand & operandMask;

    if (instr.type == InstructionType::D2_LABEL)
    {
        fixups.emplace_back(instr.label,
                            instr.token,
                            (unsigned int) bytes.size(),
                            instr.type);
    }

    // Check word aligned
    if (bytes.size() % 4 != 0)
    {
        errors.emplace_back(M20ErrorType::ALIGNMENT,
                            instr.token,
                            "Instruction must be 4 byte aligned");
        return;
    }
    bytes.push_back((char) ((code >> 24) & 0xFF));
    bytes.push_back((char) ((code >> 16) & 0xFF));
    bytes.push_back((char) ((code >> 8) & 0xFF));
    bytes.push_back((char) (code & 0xFF));
}

void m20::Assembler::generateDataLoad(const Instruction &instr)
{
    int code = 0x08000000;
    code |= getConditionNibble(instr.cond);
    code |= getLoadOpcode(instr.command);

    int hasBase = 0x01000000;
    if (instr.type == InstructionType::M_RELATIVE
        || instr.type == InstructionType::M_RELATIVE_LABEL
        || instr.type == InstructionType::M_DIRECT_INDEX)
    {
        hasBase = 0;
    }
    code |= hasBase;

    int hasImmediate = 0x02000000;
    if (instr.type == InstructionType::M_DIRECT_INDEX
        || instr.type == InstructionType::M_BASE_INDEX)
    {
        hasImmediate = 0;
    }
    code |= hasImmediate;

    int operandMask = 0x00000FFF;
    if (instr.type == InstructionType::M_RELATIVE
        || instr.type == InstructionType::M_RELATIVE_LABEL
        || instr.type == InstructionType::M_DIRECT_INDEX)
    {
        operandMask = 0x0000FFFF;
    }

    code |= (instr.rd << 16) & 0x000F0000;
    code |= (instr.rn << 12) & 0x0000F000;
    code |= instr.operand & operandMask;

    if (instr.type == InstructionType::M_RELATIVE_LABEL
        || instr.type == InstructionType::M_BASE_OFFSET_LABEL)
    {
        fixups.emplace_back(instr.label,
                            instr.token,
                            (unsigned int) bytes.size(),
                            instr.type);
    }

    // Check word aligned
    if (bytes.size() % 4 != 0)
    {
        errors.emplace_back(M20ErrorType::ALIGNMENT,
                            instr.token,
                            "Instruction must be 4 byte aligned");
        return;
    }
    bytes.push_back((char) ((code >> 24) & 0xFF));
    bytes.push_back((char) ((code >> 16) & 0xFF));
    bytes.push_back((char) ((code >> 8) & 0xFF));
    bytes.push_back((char) (code & 0xFF));
}

void m20::Assembler::generateBranch(const Instruction &instr)
{
    int code = 0x0C000000;
    code |= getConditionNibble(instr.cond);

    int link = 0x01000000;
    if (instr.command == InstructionCommand::B)
    {
        link = 0;
    }
    code |= link;

    int hasImmediate = 0x00800000;
    if (instr.type == InstructionType::B_ABSOLUTE)
    {
        hasImmediate = 0;
    }
    code |= hasImmediate;

    code |= instr.operand & 0x007FFFFF;

    if (instr.type == InstructionType::B_RELATIVE_LABEL)
    {
        fixups.emplace_back(instr.label,
                            instr.token,
                            (unsigned int) bytes.size(),
                            instr.type);
    }

    // Check word aligned
    if (bytes.size() % 4 != 0)
    {
        errors.emplace_back(M20ErrorType::ALIGNMENT,
                            instr.token,
                            "Instruction must be 4 byte aligned");
        return;
    }
    bytes.push_back((char) ((code >> 24) & 0xFF));
    bytes.push_back((char) ((code >> 16) & 0xFF));
    bytes.push_back((char) ((code >> 8) & 0xFF));
    bytes.push_back((char) (code & 0xFF));
}

void m20::Assembler::generateSwi(const Instruction &instr)
{
    int code = 0x0F000000;
    code |= getConditionNibble(instr.cond);

    code |= instr.operand & 0x00FFFFFF;

    // Check word aligned
    if (bytes.size() % 4 != 0)
    {
        errors.emplace_back(M20ErrorType::ALIGNMENT,
                            instr.token,
                            "Instruction must be 4 byte aligned");
        return;
    }
    bytes.push_back((char) ((code >> 24) & 0xFF));
    bytes.push_back((char) ((code >> 16) & 0xFF));
    bytes.push_back((char) ((code >> 8) & 0xFF));
    bytes.push_back((char) (code & 0xFF));
}

int m20::Assembler::getConditionNibble(const Conditional &cond)
{
    switch (cond)
    {
        case Conditional::EQ:
            return 0x00000000;
        case Conditional::NE:
            return 0x10000000;
        case Conditional::CS:
            return 0x20000000;
        case Conditional::CC:
            return 0x30000000;
        case Conditional::MI:
            return 0x40000000;
        case Conditional::PL:
            return 0x50000000;
        case Conditional::VS:
            return 0x60000000;
        case Conditional::VC:
            return 0x70000000;
        case Conditional::HI:
            return 0x80000000;
        case Conditional::LS:
            return 0x90000000;
        case Conditional::GE:
            return 0xA0000000;
        case Conditional::LT:
            return 0xB0000000;
        case Conditional::GT:
            return 0xC0000000;
        case Conditional::LE:
            return 0xD0000000;
        default:
            return 0xE0000000;
    }
}

int m20::Assembler::getDataOpcode(const InstructionCommand &command)
{
    switch (command)
    {
        case InstructionCommand::NOOP:
            return 0x00000000;
        case InstructionCommand::ADD:
            return 0x00100000;
        case InstructionCommand::ADC:
            return 0x00200000;
        case InstructionCommand::SUB:
            return 0x00300000;
        case InstructionCommand::SBC:
            return 0x00400000;
        case InstructionCommand::MUL:
            return 0x00500000;
        case InstructionCommand::DIV:
            return 0x00600000;
        case InstructionCommand::UDV:
            return 0x00700000;
        case InstructionCommand::OR:
            return 0x00800000;
        case InstructionCommand::AND:
            return 0x00900000;
        case InstructionCommand::XOR:
            return 0x00A00000;
        case InstructionCommand::NOR:
            return 0x00B00000;
        case InstructionCommand::BIC:
            return 0x00C00000;
        case InstructionCommand::ROR:
            return 0x00D00000;
        case InstructionCommand::LSL:
            return 0x00E00000;
        case InstructionCommand::LSR:
            return 0x00F00000;
        case InstructionCommand::ASR:
            return 0x01000000;
        case InstructionCommand::MOV:
            return 0x01100000;
        case InstructionCommand::MVN:
            return 0x01200000;
        case InstructionCommand::CMP:
            return 0x01300000;
        case InstructionCommand::CMN:
            return 0x01400000;
        case InstructionCommand::TST:
            return 0x01500000;
        case InstructionCommand::TEQ:
            return 0x01600000;
        case InstructionCommand::PUSH:
            return 0x01700000;
        case InstructionCommand::POP:
            return 0x01800000;
        case InstructionCommand::SRL:
            return 0x01900000;
        case InstructionCommand::SRS:
            return 0x01A00000;
        case InstructionCommand::HALT:
            return 0x01F00000;
        default:
            assert(false);
            return 0;
    }
}

int m20::Assembler::getLoadOpcode(const InstructionCommand &command)
{
    switch (command)
    {
        case InstructionCommand::LDR:
            return 0x00000000;
        case InstructionCommand::LDRB:
            return 0x00100000;
        case InstructionCommand::LDRH:
            return 0x00200000;
        case InstructionCommand::LDRSB:
            return 0x00300000;
        case InstructionCommand::LDRSH:
            return 0x00400000;
        case InstructionCommand::STR:
            return 0x00500000;
        case InstructionCommand::STRB:
            return 0x00600000;
        case InstructionCommand::STRH:
            return 0x00700000;
        default:
            assert(false);
            return 0;
    }
}

void m20::Assembler::updateLabels()
{
    for (const auto &global : globals)
    {
        auto i = labels.find(global.label);
        if (i == labels.end())
        {
            errors.emplace_back(M20ErrorType::LABEL,
                                global.token,
                                "Global not defined in file");
        }
        else if (externs.find(LinkLabel(global.label)) != externs.end())
        {
            errors.emplace_back(M20ErrorType::LABEL,
                                global.token,
                                "Duplicate global and extern");
        }
        else if (entryPoint == global.label)
        {
            addSymbol(SymbolType::ENTRY, i->second);
        }
        else
        {
            addSymbol(SymbolType::DEFINED, i->second);
        }
    }

    for (const auto &e : externs)
    {
        auto i = labels.find(e.label);
        if (i != labels.end())
        {
            errors.emplace_back(M20ErrorType::LABEL,
                                e.token,
                                "Extern defined in file");
        }
        else
        {
            addSymbol(SymbolType::UNDEFINED, {e.label, 0, 0});
        }
    }

    for (const auto &fixup : fixups)
    {
        size_t sectionIndex = 0;
        for (size_t i = 0; i < sections.size(); ++i)
        {
            if (sections[i].end > fixup.index)
            {
                sectionIndex = i;
                break;
            }
        }

        try
        {
            if (fixup.label == "$")
            {
                addRelocation(fixup.type, fixup.index, "");
                continue;
            }

            auto i = labels.find(fixup.label);
            if (externs.find(LinkLabel(fixup.label)) != externs.end())
            {
                addRelocation(fixup.type, fixup.index, fixup.label);
            }
            else if (i != labels.end())
            {
                if (i->second.index >= sections.at(sectionIndex).begin
                    && i->second.index < sections.at(sectionIndex).end)
                {
                    int immediate = getImmediate(fixup.index,
                                                 i->second.index,
                                                 fixup.type);
                    bytes[fixup.index] |=
                            ((char) ((immediate >> 24) & 0xFF));
                    bytes[fixup.index + 1] |=
                            ((char) ((immediate >> 16) & 0xFF));
                    bytes[fixup.index + 2] |=
                            ((char) ((immediate >> 8) & 0xFF));
                    bytes[fixup.index + 3] |=
                            ((char) (immediate & 0xFF));
                }
                else
                {
                    addSymbol(SymbolType::LOCAL, i->second);
                    addRelocation(fixup.type, fixup.index, fixup.label);
                }
            }
            else
            {
                errors.emplace_back(M20ErrorType::LABEL,
                                    fixup.token,
                                    "Undefined label (is it extern?)");
            }
        }
        catch (const std::out_of_range &e)
        {
            errors.emplace_back(M20ErrorType::SYNTAX,
                                fixup.token,
                                "Label immediate causes out of range "
                                        "value");
        }
        catch (const std::range_error &e)
        {
            errors.emplace_back(M20ErrorType::LABEL,
                                fixup.token,
                                "Label must be 4 byte aligned to be "
                                        "used as an immediate");
        }
    }
}

void m20::Assembler::addSymbol(m20::SymbolType type,
                               const Label &label)
{
    symbolTable.push_back(static_cast<char>(type));

    symbolTable.push_back((char) ((label.index >> 24) & 0xFF));
    symbolTable.push_back((char) ((label.index >> 16) & 0xFF));
    symbolTable.push_back((char) ((label.index >> 8) & 0xFF));
    symbolTable.push_back((char) (label.index & 0xFF));

    uint16_t lpos = addLabel(label.label);
    symbolTable.push_back((char) ((lpos >> 8) & 0xFF));
    symbolTable.push_back((char) (lpos & 0xFF));
}

void m20::Assembler::addRelocation(m20::InstructionType type,
                                   unsigned int address,
                                   const std::string &label)
{
    if (label != "$"
        && (type == InstructionType::B_RELATIVE_LABEL
            || type == InstructionType::DIRECTIVE_DATA_ADDR)
        && labels.find(label)->second.index % 4 != 0)
    {
        throw std::range_error("Label not 4 byte aligned");
    }

    relocationTable.push_back(static_cast<char>(type));

    relocationTable.push_back((char) ((address >> 24) & 0xFF));
    relocationTable.push_back((char) ((address >> 16) & 0xFF));
    relocationTable.push_back((char) ((address >> 8) & 0xFF));
    relocationTable.push_back((char) (address & 0xFF));

    uint16_t lpos = addLabel(label);
    relocationTable.push_back((char) ((lpos >> 8) & 0xFF));
    relocationTable.push_back((char) (lpos & 0xFF));
}

uint16_t m20::Assembler::addLabel(const std::string &label)
{
    auto i = strings.find(label);
    if (i == strings.end())
    {
        uint16_t index = stringsIndex;
        strings[label] = stringsIndex;
        ++stringsIndex;
        if (stringsIndex < index)
        {
            throw std::overflow_error("Too many labels");
        }

        for (const auto &c : label)
        {
            stringBytes.push_back(c);
        }
        stringBytes.push_back(0);

        return index;
    }
    else
    {
        return i->second;
    }
}

int m20::Assembler::getImmediate(unsigned int addr,
                                 unsigned int label,
                                 InstructionType type)
{
    bool relative = false;  // Relative or absolute
    bool relativePc = false;
    bool aligned = false;
    int bits = 0;           // Number of bits

    switch (type)
    {
        case InstructionType::D2_LABEL:
            relative = false;
            bits = 16;
            break;
        case InstructionType::B_RELATIVE_LABEL:
            relative = true;
            relativePc = true;
            aligned = true;
            bits = 23;
            break;
        case InstructionType::M_RELATIVE_LABEL:
            relative = true;
            relativePc = true;
            bits = 16;
            break;
        case InstructionType::M_BASE_OFFSET_LABEL:
            relative = true;
            bits = 12;
            break;
        case InstructionType::DIRECTIVE_DATA_ADDR:
            relative = false;
            bits = 32;
            break;
        default:
            assert(false);
            break;
    }

    auto offset = (int) label;
    if (relative)
    {
        offset = label - addr;

        if (relativePc)
        {
            offset -= 4;
        }
    }

    if (aligned && offset % 4 != 0)
    {
        throw std::range_error("Label not 4 byte aligned");
    }

    unsigned int mask = 0xFFFFFFFF;
    mask <<= bits;
    if (bits == 32)
    {
        mask = 0;
    }

    if (offset < 0)
    {
        mask = ~(mask & offset) & mask;
    }
    else
    {
        mask = (mask & offset);
    }

    if (mask)
    {
        throw std::out_of_range("Label out of bounds");
    }

    mask = 0xFFFFFFFF;
    mask >>= 32 - bits;

    if (aligned)
    {
        offset >>= 2;
    }

    return offset & mask;
}

void m20::Assembler::printErrors(const std::string &filename,
                                 std::vector<m20::M20Error> &errors,
                                 std::vector<std::string> &lines)
{
    int totalErrors = 0;
    int currentLine = 0;
    for (const auto &e : errors)
    {
        if (e.token.line == currentLine)
        {
            continue;
        }
        currentLine = e.token.line;
        ++totalErrors;

        size_t index = (e.token.line == -1
                        ? std::string::npos
                        : lines[e.token.line - 1].find_first_not_of(' ')
        );
        if (index == std::string::npos)
        {
            index = 0;
        }

        std::string line = "";
        std::stringstream spaces;
        if (e.token.line >= 1)
        {
            line = lines[e.token.line - 1].substr(index);
            index = line.find(e.token.raw);
            assert(index != std::string::npos);

            for (size_t i = 0; i < index; ++ i)
            {
                spaces << " ";
            }
        }

        std::cout << "\x1B[4m" << filename << "\x1B[0m\x1B[1m("
                  << e.token.line << ":"
                  << e.token.column << ") \x1B[31;1mError: \x1B[39;1m"
                  << e.message << "\n\x1B[0m";
        std::cout << "\t" << line << "\n";
        std::cout << "\t\x1B[31;1m" << spaces.str() << "^\x1B[0m"
                  << std::endl;
    }

    std::cout << "\n";
    std::cout << totalErrors << " error(s) found." << std::endl;
}

void m20::Assembler::printByteDump(int width, bool printLabels)
{
    std::string printlabel;
    for (int i = 0; i < bytes.size();)
    {
        if (printLabels)
        {
            auto firstlookup = labelLookup.find((unsigned int) i);
            if (printlabel.length() == 0 && firstlookup != labelLookup.end())
            {
                printlabel = firstlookup->second.label;
            }
            if (printlabel.length() > 0)
            {
                if (i > 0)
                {
                    std::cout << "\n";
                }
                std::cout << "<" << printlabel << ">:\n";
            }
        }
        std::cout << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << (i) << ": ";
        int b = ((int) bytes.size() - width > i
                 ? width
                 : (int) bytes.size() - i);
        for (int j = 0; j < width; ++j)
        {
            if (j < b)
            {
                if (printLabels)
                {
                    auto lookup = labelLookup.find((unsigned int) (i + j));
                    if (lookup != labelLookup.end())
                    {
                        if (printlabel.length() == 0)
                        {
                            b = j;
                            printlabel = lookup->second.label;
                            std::cout << std::setw(3 * (width - j))
                                      << std::setfill(' ') << "   ";
                            break;
                        }
                        else
                        {
                            printlabel = "";
                        }
                    }
                }
                std::cout << std::setw(2) << std::setfill('0');
                std::cout << (static_cast<unsigned int>(bytes[i + j])
                              & 0xFF);
                std::cout << " ";
            }
            else
            {
                std::cout << "   ";
            }
        }
        for (int j = 0; j < b; ++j)
        {
            char c = bytes[i + j];
            if (isprint(c))
            {
                std::cout << c;
            }
            else
            {
                std::cout << ".";
            }
        }
        i += b;
        std::cout << "\n";
    }
    std::cout << std::flush;
}
