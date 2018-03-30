/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Syntax Parser for M20 Assembly.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_PARSER_H
#define M20_ASSEMBLY_PARSER_H

#include <vector>

#include "Instruction.h"
#include "Token.h"

namespace m20
{
    enum class M20ErrorType : unsigned int
    {
        SYNTAX,
        DIRECTIVE,
        SECTION,
        LABEL,
        ALIGNMENT
    };

    struct M20Error
    {
        const M20ErrorType type;
        const Token &token;
        const std::string message;

        M20Error(M20ErrorType type, const Token &token,
                   const std::string &message);
    };

    class Parser
    {
    public:
        /**
         * Initializes new Parser
         * @param tokens
         */
        Parser(const std::vector<const Token> &tokens);

        /**
         * Parses a vector of tokens into a vector of statements
         * @param tokens
         * @param statements
         * @return
         */
        bool parse();

        /**
         * Returns the number of errors generated by the parse
         * @return
         */
        size_t getNumErrors()
        {
            return errors.size();
        }

        /**
         * Returns the parsing errors generated when parsing
         * @return
         */
        std::vector<M20Error> &getErrors()
        {
            return errors;
        }

        /**
         * Returns the parse items generated when parsing
         * @return
         */
        std::vector<Instruction> &getItems()
        {
            return items;
        }

        /**
         * Prints the errors collected from parsing
         */
        void printErrors();

    private:
        void parseStatement(const Token &current);

        inline const Token &getCurrent();
        inline const Token &next();
        inline const Token &peek();
        inline bool hasNext();
        inline const Token &accept(TokenType type);
        inline void expect(TokenType type);

        inline size_t parseNumber(const Token &num,
                                  int maxBytes);
        inline unsigned int parseArgNumber(const Token &num);
        inline unsigned int parseInt(const Token &num,
                                     int maxBits);
        inline void parseDataNumber(const Token &num,
                                    int maxBytes,
                                    std::vector<char> &bytes);
        inline void parseDataString(const std::string &str,
                                    std::vector<char> &bytes);

        inline unsigned int parseRegister(const std::string &str,
                                          bool status = false);

        inline Instruction get3Arg(const Token &command);
        inline Instruction get2Arg(const Token &command);
        inline Instruction get1Arg(const Token &command);
        inline Instruction getMem(const Token &command);

        inline static bool streqi(const std::string &l, const std::string &r);

        size_t index;
        const std::vector<const Token> &tokens;
        std::vector<Instruction> items;
        std::vector<M20Error> errors;
    };
}

#endif // M20_ASSEMBLY_PARSER_H
