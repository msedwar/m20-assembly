/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Assembler for M20 Assembly.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_ASSEMBLER_H
#define M20_ASSEMBLY_ASSEMBLER_H

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "Instruction.h"
#include "Lexer.h"
#include "Parser.h"
#include "Token.h"
#include "Utils.h"

namespace m20
{
    enum class SymbolType : char
    {
        UNDEFINED = 0,
        DEFINED   = 1,
        LOCAL     = 2,
        ENTRY     = 3
    };

    /**
     * Handles assembly of M20 notational code into an object file
     */
    class Assembler
    {
    public:
        /**
         * Initializes new Assembler
         */
        Assembler()
            : hasEntrypoint(false)
        {
            //
        }

        /**
         * Reads input file, assembles the M20 Assembly code, and ouputs the
         *  object file.
         * @param inputFile Input file (.as)
         * @param outputFile Output file (.obj)
         */
        bool assemble(const std::string &inputFileName,
                      const std::string &outputFileName);

        static const Token BEGIN_TOKEN;

    private:
        struct LinkLabel
        {
            const std::string label;
            const Token &token;

            LinkLabel(const std::string &label)
                : label(label), token(Assembler::BEGIN_TOKEN)
            {
                //
            }

            LinkLabel(const std::string &label, const Token &token)
                    : label(label), token(token)
            {
                //
            }

            bool operator==(const LinkLabel &other) const
            {
                return label == other.label;
            }

            bool operator==(const std::string &other) const
            {
                return label == other;
            }
        };

        struct LinkLabelHasher
        {
            size_t operator() (const m20::Assembler::LinkLabel &obj) const
            {
                return std::hash<std::string>()(obj.label);
            }
        };

        struct Label
        {
            std::string label;
            unsigned int index;
            unsigned int section;
        };

        struct LabelFixup
        {
            const std::string label;
            const Token &token;
            const unsigned int index;
            const InstructionType type;

            LabelFixup(const std::string &label,
                       const Token &token,
                       unsigned int index,
                       const InstructionType type)
                    : label(label),
                      token(token),
                      index(index),
                      type(type)
            {
                //
            }
        };

        struct Section
        {
            const Token &token;
            unsigned int begin;
            unsigned int end;
            bool text;

            Section(const Token &token, unsigned int begin, bool text)
                : token(token), begin(begin), end(0), text(text)
            {
                //
            }
        };

        std::vector<M20Error> errors;

        std::map<std::string, Label> labels;
        std::map<unsigned int, Label> labelLookup;
        std::vector<LabelFixup> fixups;
        std::vector<Section> sections;
        std::vector<LinkLabel> globals;
        std::unordered_set<LinkLabel, LinkLabelHasher> externs;
        std::vector<char> bytes;
        std::vector<char> symbolTable;
        std::vector<char> relocationTable;

        uint16_t stringsIndex = 0;
        std::map<std::string, uint16_t> strings;
        std::vector<char> stringBytes;

        std::string entryPoint;
        bool hasEntrypoint;

        void generate(std::vector<Instruction> &items);
        void generateDataProcessing(const Instruction &instr);
        void generateDataLoad(const Instruction &instr);
        void generateBranch(const Instruction &instr);
        void generateSwi(const Instruction &instr);

        int getConditionNibble(const Conditional &cond);
        int getDataOpcode(const InstructionCommand &command);
        int getLoadOpcode(const InstructionCommand &command);

        void updateLabels();
        void addSymbol(SymbolType type,
                       const Label &label);
        void addRelocation(InstructionType type,
                           unsigned int address,
                           const std::string &label);
        uint16_t addLabel(const std::string &label);
        int getImmediate(unsigned int addr,
                         unsigned int label,
                         InstructionType type);

        void printErrors(const std::string &filename,
                         std::vector<M20Error> &errors,
                         std::vector<std::string> &lines);

        void printByteDump(int width, bool printLabels);
    };
}

#endif // M20_ASSEMBLY_ASSEMBLER_H
