/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 22, 2018
 * Description:
 *      Linker for M20 Assembly object files.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_LINKER_H
#define M20_ASSEMBLY_LINKER_H

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "Assembler.h"
#include "Utils.h"

namespace m20
{
    struct LinkerError
    {
        std::string file;
        std::string message;
        std::string label;

        LinkerError(const std::string &file,
                    const std::string &message,
                    const std::string &label)
            : file(file), message(message), label(label)
        {
            //
        }
    };

    /**
     * Handles linking of M20 object files into an executable
     */
    class Linker
    {
    public:
        /**
         * Initializes new Linker
         */
        Linker() = default;

        /**
         * Links specified files into executable
         * @param files Vector of object files
         * @param executable Name of executable to create
         * @return
         */
        bool link(const std::vector<const std::string> &files,
                  const std::string &executable);

    private:
        struct Section
        {
            std::string file;
            unsigned int begin;
            unsigned int end;
            unsigned int address;
            bool text;
            char *bytes;

            Section(const std::string &file,
                    unsigned int begin,
                    unsigned int end,
                    bool text)
                    : file(file),
                      begin(begin),
                      end(end),
                      address(0),
                      text(text),
                      bytes(nullptr)
            {
                //
            }

            void deleteBytes()
            {
                assert(bytes);
                delete[] bytes;
                bytes = nullptr;
            }

            unsigned int getSize() const
            {
                return end - begin;
            }
        };

        struct Symbol
        {
            std::string label;
            unsigned int section;
            unsigned int address;
            SymbolType type;

            Symbol(const std::string &label,
                   unsigned int section,
                   unsigned int address,
                   SymbolType type)
                : label(label),
                  section(section),
                  address(address),
                  type(type)
            {
                //
            }

            bool operator==(const Symbol &other) const
            {
                return label == other.label;
            }

            bool operator==(const std::string &other) const
            {
                return label == other;
            }
        };

        struct SymbolHasher
        {
            size_t operator() (const m20::Linker::Symbol &obj) const
            {
                return std::hash<std::string>()(obj.label);
            }
        };

        struct Relocation
        {
            std::string label;
            unsigned int section;
            unsigned int address;
            InstructionType type;

            Relocation(const std::string &label,
                   unsigned int section,
                   unsigned int address,
                   InstructionType type)
                    : label(label),
                      section(section),
                      address(address),
                      type(type)
            {
                //
            }
        };

        std::vector<LinkerError> errors;
        std::vector<char> bytes;

        std::vector<Section> sections;
        std::map<std::string, std::string> definedSymbols;
        std::map<std::string,
                 std::unordered_set<Symbol, SymbolHasher> > symbols;
        std::map<unsigned int, std::vector<Relocation> > relocations;

        void readFiles(const std::vector<const std::string> &files);

        void fixupSection(unsigned int index);
        void fixupSymbol(InstructionType type,
                         unsigned int current,
                         unsigned int address);
        int getImmediate(unsigned int addr,
                         unsigned int label,
                         InstructionType type);

        void printErrors();
    };
}

#endif // M20_ASSEMBLY_LINKER_H
