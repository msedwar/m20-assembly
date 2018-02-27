/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Syntax lexer for M20 Assembly.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_LEXER_H
#define M20_ASSEMBLY_LEXER_H

#include <cassert>
#include <regex>
#include <string>
#include <vector>

#include "Token.h"

namespace m20
{
    /**
     * Syntax lexer for M20 Assembly
     */
    class Lexer
    {
    public:
        /**
         * Takes an input string and lexes it into a stream of tokens
         * @param in Input stream from which to create tokens
         * @param tokens Vector of created tokens
         */
        static void lex(const std::string &in,
                        std::vector<const Token> &tokens);
    };
}

#endif // M20_ASSEMBLY_LEXER_H
