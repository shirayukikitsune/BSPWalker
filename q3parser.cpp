#include "q3parser.h"

Q3Parser::Q3Parser(char const * data)
    : first(data), current(data)
{

}

bool Q3Parser::isWhitespace(const char chr)
{
    return chr == ' ' || chr == '\t' || chr== '\n';
}

bool Q3Parser::isLiteralCharacter(const char chr)
{
    return  (chr >= 'a' && chr <= 'z') ||
            (chr >= 'A' && chr <= 'Z') ||
            (chr >= '0' && chr <= '9') ||
            (chr == '/' || chr == '.' || chr == '_');
}

Q3TokenType Q3Parser::next()
{
    currentToken.clear();

    // Skip whitespaces
    while (isWhitespace(*current)) {
        ++current;
    }

    char value = *(current++);
    // Check for special characters
    switch (value) {
    case '\0': // Null Terminator
        return Q3TOK_EOF;

    case '/': // Comment or division
        // To check the type of the token, checking the next character is required
        switch (current[1]) {
        case '\0':
            return Q3TOK_EOF;
        case '/': // Found a comment (//), skip until end of line (or file)
            while (*(current++) != '\n') {
                if (*current == '\0')
                    return Q3TOK_EOF;
            }
            return Q3TOK_COMMENT;
        }
        break;

    case '\n':
        return Q3TOK_EOL;

    case '{':
        return Q3TOK_LIST_START;

    case '}':
        return Q3TOK_LIST_END;

    case '"': // String
        // Read to next " character (or EOF)
        while (*current != '"') {
            currentToken.append(*(current++));

            if (*current == '\0') {
                currentToken.clear();
                return Q3TOK_EOF;
            }
        }
        // Skip the closing "
        ++current;
        return Q3TOK_STRING;
    }

    currentToken.append(value);

    // Not a special character, then it is a literal; read to next valid literal character
    while (isLiteralCharacter(*current)) {
        currentToken.append(*(current++));

        // Check for EOF
        if (*current == '\0') {
            currentToken.clear();
            return Q3TOK_EOF;
        }
    }

    return Q3TOK_LITERAL;
}
