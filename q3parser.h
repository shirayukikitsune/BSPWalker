#ifndef Q3PARSER_H
#define Q3PARSER_H

#include <QString>

enum Q3TokenType {
    Q3TOK_EOF,
    Q3TOK_MATH_DIVIDE,
    Q3TOK_COMMENT,
    Q3TOK_EOL,
    Q3TOK_LIST_START,
    Q3TOK_LIST_END,
    Q3TOK_STRING,
    Q3TOK_LITERAL,
};

/**
 * @brief A parser for Quake3 entity and shader files
 */
class Q3Parser
{
public:
    /**
     * @brief Constructs the parser
     * @param data A null-terminated string containing the file contents
     */
    Q3Parser(char const *data);

    /**
     * @brief Parses the next token of the buffer
     * @return The type of the current token
     */
    Q3TokenType next();

    /**
     * @brief Returns the current available token, or an empty string if none
     */
    QString getCurrentToken() { return currentToken; }

private:
    char const * first;
    char const * current;

    QString currentToken;

    bool isWhitespace(const char chr);
    bool isLiteralCharacter(const char chr);
};

#endif // Q3PARSER_H
