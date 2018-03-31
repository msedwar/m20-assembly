/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 17, 2018
 * Description:
 *      Syntax lexer for M20 Assembly. (Implementation)
 * =============================================================================
 */

#include <cctype>

#include "Instruction.h"
#include "Lexer.h"

void m20::Lexer::lex(const std::string &in,
                     std::vector<const m20::Token> &tokens)
{
    std::string input = in;
    int line = 1;
    int column = 0;
    while (!input.empty())
    {
        size_t currentMatch = 0;
        std::smatch match;
        Token token;
        token.line = line;
        token.column = column;
        for (unsigned int i = 1; i < TOKEN_COUNT; ++i)
        {
            if (std::regex_search(input, match, TOKEN_REGEX[i],
                                  std::regex_constants::match_continuous)
                    && match[0].length() > currentMatch)
            {
                token.raw = match[0];
                token.type = static_cast<TokenType>(i);
                currentMatch = (size_t) match[0].length();
            }
        }

        if (currentMatch == 0)
        {
            currentMatch = input.find_first_of(" \t\n\r");
            token.raw = input.substr(0, currentMatch);
            if (currentMatch == std::string::npos)
            {
                input = "";
                continue;
            }
        }

        if (token.type == TokenType::D3_INSTR
                && std::regex_search(token.raw, match, STATUS_UPDATE_REGEX))
        {
            token.updateStatus = true;
            token.raw = token.raw.substr(0, token.raw.length() - 2);
        }
        else
        {
            token.updateStatus = false;
        }

        if ((token.type == TokenType::D3_INSTR
            || token.type == TokenType::D2_INSTR
            || token.type == TokenType::D1_INSTR
            || token.type == TokenType::EMPTY_INSTR
            || token.type == TokenType::MEM_INSTR
            || token.type == TokenType::INSTRUCTION)
            && std::regex_search(token.raw, match, CONDITIONAL_REGEX))
        {
            if (token.raw.length() >= 5
                || tolower(token.raw[0]) == 'b')
            {
                std::string cond = token.raw.substr(
                        token.raw.length() - 2,
                        token.raw.length());
                cond[0] = (char) tolower(cond[0]);
                cond[1] = (char) tolower(cond[1]);

                if (cond == "eq")
                {
                    token.condition = m20::Conditional::EQ;
                }
                else if (cond == "ne")
                {
                    token.condition = m20::Conditional::NE;
                }
                else if (cond == "cs")
                {
                    token.condition = m20::Conditional::CS;
                }
                else if (cond == "cc")
                {
                    token.condition = m20::Conditional::CC;
                }
                else if (cond == "mi")
                {
                    token.condition = m20::Conditional::MI;
                }
                else if (cond == "pl")
                {
                    token.condition = m20::Conditional::PL;
                }
                else if (cond == "vs")
                {
                    token.condition = m20::Conditional::VS;
                }
                else if (cond == "vc")
                {
                    token.condition = m20::Conditional::VC;
                }
                else if (cond == "hi")
                {
                    token.condition = m20::Conditional::HI;
                }
                else if (cond == "ls")
                {
                    token.condition = m20::Conditional::LS;
                }
                else if (cond == "ge")
                {
                    token.condition = m20::Conditional::GE;
                }
                else if (cond == "lt")
                {
                    token.condition = m20::Conditional::LT;
                }
                else if (cond == "gt")
                {
                    token.condition = m20::Conditional::GT;
                }
                else if (cond == "le")
                {
                    token.condition = m20::Conditional::LE;
                }
                else if (cond == "al")
                {
                    token.condition = m20::Conditional::AL;
                }
                else
                {
                    token.type = TokenType::INVALID;
                }

                if (token.type != TokenType::INVALID)
                {
                    token.raw = token.raw.substr(0,
                            token.raw.length() - 2);
                }
            }
        }
        tokens.push_back(token);

        if (token.type == TokenType::WHITESPACE)
        {
            int newlines = (int) std::count(token.raw.begin(),
                                         token.raw.end(), '\n');
            if (newlines > 0)
            {
                line += newlines;
                column = (int) (token.raw.length()
                                - token.raw.find_last_of('\n') - 1);
            }
            else
            {
                column += (int) currentMatch;
            }
        }
        else
        {
            column += (int) currentMatch;
        }

        if (token.type == TokenType::WHITESPACE
                || token.type == TokenType::COMMENT)
        {
            tokens.pop_back();
        }

        input = input.substr(currentMatch);
    }
}
