/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 22, 2018
 * Description:
 *      Linker for M20 Assembly object files. (Implementation)
 * =============================================================================
 */

#include <fstream>
#include <iomanip>
#include <iostream>

#include "Linker.h"

bool m20::Linker::link(const std::vector<const std::string> &files,
                       const std::string &executable)
{
    readFiles(files);

    for (const auto &fileSymbols : symbols)
    {
        for (const auto &symbol : fileSymbols.second)
        {
            if (symbol.type == SymbolType::UNDEFINED
                && definedSymbols.find(symbol.label) == definedSymbols.end())
            {
                errors.emplace_back(sections[symbol.section].file,
                                    "Undefined symbol",
                                    symbol.label);
            }
        }
    }

    if (!errors.empty())
    {
        printErrors();
        return false;
    }

    std::vector<unsigned int> textSections;
    std::vector<unsigned int> dataSections;
    size_t totalBytes = 0;
    for (unsigned int i = 0; i < sections.size(); ++i)
    {
        if (sections[i].text)
        {
            textSections.push_back(i);
        }
        else
        {
            dataSections.push_back(i);
        }
        totalBytes += sections[i].getSize();
    }

    bytes.reserve(totalBytes + 3);
    unsigned int address = 0;
    for (const auto &i : textSections)
    {
        sections[i].address = address;
        for (unsigned int j = 0; j < sections[i].getSize(); ++j)
        {
            bytes.push_back(sections[i].bytes[j]);
        }
        address += sections[i].getSize();
    }
    for (const auto &i : dataSections)
    {
        sections[i].address = address;
        for (unsigned int j = 0; j < sections[i].getSize(); ++j)
        {
            bytes.push_back(sections[i].bytes[j]);
        }
        address += sections[i].getSize();
    }

    for (const auto &i : textSections)
    {
        fixupSection(i);
    }
    for (const auto &i : dataSections)
    {
        fixupSection(i);
    }

    // Cleanup heap memory
    for (auto &section : sections)
    {
        section.deleteBytes();
    }

    for (auto i = (unsigned int) (bytes.size() % 4); i < 4; ++i)
    {
        bytes.push_back(0);
    }

    std::ofstream outfile(executable, std::ios::out | std::ios::binary);
    assert(outfile.is_open());

    outfile.write(bytes.data(), bytes.size());

    outfile.close();

    return true;
}

void m20::Linker::readFiles(const std::vector<const std::string> &files)
{
    unsigned int sectionOffset = 0;
    for (const auto &fname : files)
    {
        std::ifstream infile(fname, std::ios::in | std::ios::binary);
        assert(infile.is_open());

        char buffer[4];

        infile.read(buffer, 4);
        unsigned int magicBytes = bytesToInt(buffer);
        assert(magicBytes == 0x7F4D3230);

        infile.read(buffer, 4);
        unsigned int version = bytesToInt(buffer);
        assert(version == 1);

        infile.read(buffer, 4);
        unsigned int sectionSize = bytesToInt(buffer);
        infile.read(buffer, 4);
        unsigned int bytesSize = bytesToInt(buffer);
        infile.read(buffer, 4);
        unsigned int stringsSize = bytesToInt(buffer);
        infile.read(buffer, 4);
        unsigned int symbolTableSize = bytesToInt(buffer);
        infile.read(buffer, 4);
        unsigned int relocationTableSize = bytesToInt(buffer);

        unsigned int begin = 0;
        for (unsigned int i = 0; i < sectionSize; ++i)
        {
            infile.read(buffer, 1);
            bool text = buffer[0] == (char) 0xFF;
            infile.read(buffer, 4);
            unsigned int end = bytesToInt(buffer);
            sections.emplace_back(fname, begin, end, text);
            begin = end;
        }
        assert(begin == bytesSize);

        for (unsigned int i = 0; i < sectionSize; ++i)
        {
            Section &section = sections.at(sectionOffset + i);
            section.bytes = new char[section.getSize()];
            infile.read(section.bytes, section.getSize());
        }

        std::vector<std::string> strings;
        auto *raw = new char[stringsSize];
        infile.read(raw, stringsSize);
        const char *constRaw = raw;

        unsigned int strIndex = 0;
        while (strIndex < stringsSize)
        {
            strings.emplace_back(constRaw + strIndex);
            while (constRaw[strIndex] != 0)
            {
                ++strIndex;
            }
            ++strIndex;
        }

        delete[] raw;

        for (unsigned int i = 0; i < symbolTableSize / 7; ++i)
        {
            char type;
            infile.read(&type, 1);
            infile.read(buffer, 4);
            unsigned int address = bytesToInt(buffer);
            infile.read(buffer, 2);
            unsigned short string = bytesToShort(buffer);

            unsigned int section = 0;
            for (unsigned int j = sectionOffset; j < sections.size(); ++j)
            {
                if (address < sections[j].end)
                {
                    section = j;
                    break;
                }
            }

            SymbolType t = static_cast<SymbolType>(type);

            if (symbols.find(fname) == symbols.end())
            {
                symbols[fname] = std::unordered_set<Symbol, SymbolHasher>();
            }
            symbols[fname].emplace(strings[string],
                                   section,
                                   address,
                                   t);

            if (t == SymbolType::DEFINED
                || t == SymbolType::ENTRY)
            {
                if (definedSymbols.find(strings[string])
                    != definedSymbols.end())
                {
                    errors.emplace_back(fname,
                                        "Duplicate global symbol",
                                        strings[string]);
                }
                else
                {
                    definedSymbols.emplace(strings[string], fname);
                }
            }
        }

        for (unsigned int i = 0; i < relocationTableSize / 7; ++i)
        {
            char type;
            infile.read(&type, 1);
            infile.read(buffer, 4);
            unsigned int address = bytesToInt(buffer);
            infile.read(buffer, 2);
            unsigned short string = bytesToShort(buffer);

            unsigned int section = 0;
            for (unsigned int j = sectionOffset; j < sections.size(); ++j)
            {
                if (address < sections[j].end)
                {
                    section = j;
                    break;
                }
            }

            if (relocations.find(section) == relocations.end())
            {
                relocations[section] = std::vector<Relocation>();
            }
            relocations[section].emplace_back(strings[string],
                                              section,
                                              address,
                                              static_cast<InstructionType>(
                                                      type
                                              )
            );
        }

        sectionOffset += sectionSize;
        infile.close();
    }
}

void m20::Linker::fixupSection(unsigned int index)
{
    assert(index >= 0 && index < sections.size());

    const Section &section = sections[index];
    const auto &fileSymbols = symbols[section.file];

    for (const auto &reloc : relocations[index])
    {
        auto ifs = fileSymbols.find({reloc.label, 0, 0, SymbolType::UNDEFINED});
        auto ids = definedSymbols.find(reloc.label);
        try
        {
            if (ifs != fileSymbols.end()
                && ifs->type == SymbolType::LOCAL)
            {
                fixupSymbol(
                        reloc.type,
                        section.address + (reloc.address - section.begin),
                        (unsigned int)
                                ((int64_t) sections[ifs->section].address
                                 + ((int64_t) ifs->address
                                    - (int64_t) sections[ifs->section].begin))
                );
            }
            else if (ids != definedSymbols.end())
            {
                const auto symbolIt = symbols.find(ids->second);
                assert(symbolIt != symbols.end());
                const auto symbol = symbolIt->second.find(
                        {reloc.label, 0, 0, SymbolType::UNDEFINED}
                );
                assert(symbol != symbolIt->second.end());
                const Symbol &other = *symbol;

                fixupSymbol(
                        reloc.type,
                        section.address + (reloc.address - section.begin),
                        (unsigned int)
                                ((int64_t) sections[other.section].address
                                 + ((int64_t) other.address
                                    - (int64_t) sections[other.section].begin))
                );
            }
            else
            {
                errors.emplace_back(section.file,
                                    "Undefined symbol",
                                    reloc.label);
            }
        }
        catch (const std::range_error &e)
        {
            errors.emplace_back(section.file,
                                "Symbol not 4 byte aligned",
                                reloc.label);
        }
        catch (const std::out_of_range &e)
        {
            errors.emplace_back(section.file,
                                "Symbol out of bounds",
                                reloc.label);
        }
    }
}

void m20::Linker::fixupSymbol(InstructionType type, unsigned int current,
                              unsigned int address)
{
    int instr = 0;
    instr |= ((unsigned int) bytes[current] & 0xFF) << 24;
    instr |= ((unsigned int) bytes[current + 1] & 0xFF) << 16;
    instr |= ((unsigned int) bytes[current + 2] & 0xFF) << 8;
    instr |= ((unsigned int) bytes[current + 3] & 0xFF);
    instr |= getImmediate(current, address, type);
    bytes[current] = (char) ((instr >> 24) & 0xFF);
    bytes[current + 1] = (char) ((instr >> 16) & 0xFF);
    bytes[current + 2] = (char) ((instr >> 8) & 0xFF);
    bytes[current + 3] = (char) (instr & 0xFF);
}

int m20::Linker::getImmediate(unsigned int addr,
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

void m20::Linker::printErrors()
{
    for (const auto &error : errors)
    {
        std::cout << error.file << ": "
                  << error.message << " ("
                  << error.label << ")" << std::endl;
    }
}
