/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Definitions of Token and TokenType used when assembling M20.
 *      (Implementation)
 * =============================================================================
 */

#include "Token.h"

m20::Token::Token()
    : raw(""),
      type(m20::TokenType::INVALID),
      condition(m20::Conditional::AL),
      line(-1),
      column(-1)
{
    //
}

std::ostream &m20::operator<<(std::ostream &os, const m20::Token &token)
{
    os << "\"" << token.raw
       << "\"<"
       << TOKEN_NAMES[static_cast<unsigned int>(token.type)]
       << ">(" << token.line
       << ":" << token.column << ")";
    return os;
}

const std::string& m20::getTokenTypeName(const TokenType type)
{
    return TOKEN_NAMES[static_cast<unsigned int>(type)];
}
