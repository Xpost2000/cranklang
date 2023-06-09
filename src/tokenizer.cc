#include "crank-cpp-runtime/crank_preamble.h"
#include "tokenizer.h"
#include <cassert>

char escape(char what) {
    switch (what) { /* most of the C escape codes here. */
        case 'e': return '\e';
        case 'a': return '\a';
        case 'b': return '\b';
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\'': return '\'';
        case '\"': return '\"';
        case '\\': return '\\';
        case '\?': return '\?';
        case '0': return  '\0';
    }

    return 0;
}

bool valid_hex_character(char c) {
    switch (c) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            return true;
    }
    return false;
}
bool is_good_for_identifier(char c) {
    return (isalnum(c) || c == '_' || c == '$');
}

Token Tokenizer_State::peek_next() {
    int now = read_cursor;
    Token result = read_next();
    read_cursor = now;
    return result;
}

int hex_char_value(char c) {
    if (c >= '0' && c <= '9') return (int)c - '0';
    else {
        if (c >= 'a' && c <= 'f') return (int)c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') return (int)c - 'A' + 10;
    }
    return -1;
}

uint64_t uinteger_pow(int x, int b) {
    uint64_t result = 1;
    for (int i = 0; i < b; ++i) result *= x;
    return result;
}
uint64_t interpret_hex_string(char* hex_string, int length) {
    uint64_t result = 0;
    int i = length;
    while (i) {
        int hex_value = hex_char_value(hex_string[length-i]);
        result += hex_value * uinteger_pow(16, (i-1));
        i--;
    }
    return result;
}

uint64_t interpret_binary_string(char* binary_string, int length) {
    uint64_t result = 0;
    int i = length;
    while (i) {
        if (binary_string[length - i] == '1')
            result |= (1 << (i-1));
        i--;
    }
    return result;
}

Token Tokenizer_State::read_next() {
    while (!finished()) {
        int  start_of_token = read_cursor;
        char current        = source[read_cursor++];

        switch (current) {
            /* skip whitespace */
            case '\t':
            case '\r':
            case '\n':
            case ' ': {
                if (current == '\n') line += 1;
            } break;

            case '\'': {
                char first = source[read_cursor++];

                Token character_token = {};
                character_token.col_found  = read_cursor;
                character_token.line_found = line;
                character_token.type = TOKEN_CHARACTER;

                if (first == '\\') {
                    char second = source[read_cursor++];
                    char escape_result = escape(second);
                    assert(escape_result && "Hmm, I don't support escaping this...");
                    character_token.valuechar = escape_result;
                } else if (first == '\'') {
                    /* not escaped but this is bad? */
                    character_token.valuechar = 0;
                } else  {
                    char second = source[read_cursor++];
                    character_token.valuechar = first;
                }

                // read_cursor++; // skip quote
                character_token.string = source.substr(start_of_token+1, (read_cursor-1)-character_token.col_found);
                return character_token;
            } break;

            case '\"': {
                /* technically I don't have to process this here but okay. */
                int string_start = read_cursor;

                Token string_token = {};
                string_token.col_found  = read_cursor;
                string_token.line_found = line;
                string_token.type = TOKEN_STRING;

                while (source[read_cursor] != '\"') {
                    if (source[read_cursor] == '\\') {
                        // escape 
                        read_cursor++;
                        string_token.stringvalue += escape(source[read_cursor]);
                    } else {
                        string_token.stringvalue += source[read_cursor];
                    }
                    read_cursor++;
                }
                string_token.string = source.substr(string_start, (read_cursor-1)-start_of_token);
                read_cursor++; // skip quote
                // keep the original string for useful things
                // like transpiling where the escaped string means absolutely
                // nothing.
                return string_token;
            } break;

                /* No these pairs are not that clever lol*/
            case ',': {
                Token result      = {};
                result.type       = TOKEN_COMMA;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '.': {
                Token result      = {};
                result.type       = TOKEN_DOT;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case ':': {
                Token result      = {};
                result.type       = TOKEN_COLON;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case ';': {
                Token result      = {};
                result.type       = TOKEN_SEMICOLON;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;

            case '(': {
                Token result      = {};
                result.type       = TOKEN_LEFT_PARENTHESIS;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case ')': {
                Token result      = {};
                result.type       = TOKEN_RIGHT_PARENTHESIS;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;

            case '[': {
                Token result      = {};
                result.type       = TOKEN_LEFT_SQUARE_BRACE;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case ']': {
                Token result      = {};
                result.type       = TOKEN_RIGHT_SQUARE_BRACE;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;

            case '{': {
                Token result      = {};
                result.type       = TOKEN_LEFT_CURLY_BRACE;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '}': {
                Token result      = {};
                result.type       = TOKEN_RIGHT_CURLY_BRACE;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;


            case '+': {
                Token result      = {};
                result.type       = TOKEN_ADD;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_ADDEQUAL;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '-': {
                Token result      = {};
                result.type       = TOKEN_SUB;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_SUBEQUAL;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '/': {
                if (peek_character() == '*' || peek_character() == '/')  {
                    // comment.
                    bool single_line_comment = peek_character() == '/';
                    next_character();
                    
                    Token comment = {};
                    comment.type = TOKEN_COMMENT;
                    comment.col_found  = read_cursor;
                    comment.line_found = line;

                    int start_of_comment = read_cursor;
                    int end_of_comment   = start_of_comment;
                    if (single_line_comment) {
                        while (!finished() && source[read_cursor] != '\n') {
                            next_character();
                        }
                        end_of_comment = read_cursor;
                    } else {
                        while (!finished()) {
                            char first = next_character();
                            if (first == '*') {
                                if (peek_character() == '/')  {
                                    end_of_comment = read_cursor-1; 
                                    next_character();
                                    comment.string     = source.substr(start_of_comment, end_of_comment-start_of_comment);
                                    return comment;
                                }
                            }
                        }
                        end_of_comment = read_cursor;
                    }
                    comment.string     = source.substr(start_of_comment, end_of_comment-start_of_comment);

                    continue;
                    // This leads to weird parsing problems right now, so
                    // I don't advice it for myself right now.
                    
                    // NOTE: just add an "ignore-comment" flag or something
                    // return comment;
                } else {
                    Token result      = {};
                    result.type       = TOKEN_DIV;
                    if (peek_character() == '=') {
                        next_character();
                        result.type = TOKEN_DIVEQUAL;
                    }
                    result.col_found  = read_cursor;
                    result.line_found = line;
                    result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                    return result;
                }

            } break;
            case '*': {
                Token result      = {};
                result.type       = TOKEN_MUL;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_MULEQUAL;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '%': {
                Token result      = {};
                result.type       = TOKEN_MOD;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_MODEQUAL;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;

            case '!': {
                Token result      = {};
                result.type       = TOKEN_NOT;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_NOTEQUAL;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '^': {
                Token result      = {};
                result.type       = TOKEN_BITXOR;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '~': {
                Token result      = {};
                result.type       = TOKEN_BITNOT;
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '=': {
                Token result      = {};
                result.type       = TOKEN_EQUAL;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_EQUALITY;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;

            case '&': {
                if (peek_character() == '&') {
                    next_character();
                    Token result      = {};
                    result.type       = TOKEN_AND;
                    result.col_found  = read_cursor;
                    result.line_found = line;
                    result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                    return result;
                } else {
                    Token result      = {};
                    result.type       = TOKEN_BITAND;
                    result.col_found  = read_cursor;
                    result.line_found = line;
                    result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                    return result;
                }
                assert(!"Error cannot parse!");
                return {};
            } break;
            case '|': {
                if (peek_character() == '|') {
                    next_character();
                    Token result      = {};
                    result.type       = TOKEN_OR;
                    result.col_found  = read_cursor;
                    result.line_found = line;
                    result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                    return result;
                } else {
                    Token result      = {};
                    result.type       = TOKEN_BITOR;
                    result.col_found  = read_cursor;
                    result.line_found = line;
                    result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                    return result;
                }
                assert(!"Error cannot parse!");
                return {};
            } break;

            case '<': {
                Token result      = {};
                result.type       = TOKEN_LT;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_LTE;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;
            case '>': {
                Token result      = {};
                result.type       = TOKEN_GT;
                if (peek_character() == '=') {
                    next_character();
                    result.type = TOKEN_GTE;
                }
                result.col_found  = read_cursor;
                result.line_found = line;
                result.string     = source.substr(start_of_token, read_cursor-start_of_token);
                return result;
            } break;

            default: {
                // symbol or number.
                // TODO: more error handling. Don't care right now.
                if (isdigit(current)) {
                    // try to special case for different literals
                    Token result         = {};
                    int   start_of_digit = read_cursor-1;
                    result.col_found     = read_cursor-1;
                    result.line_found    = line;
                    if (current == '0') {
                        // 0x = hex
                        // 0b = binary
                        // no octal literals
                        switch (peek_character()) {
                            case 'X':
                            case 'x': { // parse hexadecimal
                                next_character();
                                char hex_string[16] = {};
                                for (int i = 0; i < 16; ++i) hex_string[i] = '0';

                                int length = 0;
                                while (is_good_for_identifier(peek_character())) {
                                    char c = next_character();
                                    assert(valid_hex_character(c) && "Invalid characters for hexadecimal literal");
                                    hex_string[length++] = c;
                                }

                                result.string = source.substr(start_of_digit, read_cursor-start_of_digit);
                                result.type = TOKEN_NUMBERINT;
                                result.uvalue32 = interpret_hex_string(hex_string, length);
                                return result;
                            } break;
                            case 'B':
                            case 'b': { // parse binary
                                next_character();
                                char binary_string[64] = {};
                                for (int i = 0; i < 64; ++i) binary_string[i] = '0';

                                int length = 0;
                                while (is_good_for_identifier(peek_character())) {
                                    char c = next_character();
                                    assert((c == '0' || c == '1') && "Invalid characters for binary literal");
                                    binary_string[length++] = c;
                                }

                                result.string = source.substr(start_of_digit, read_cursor-start_of_digit);
                                result.type = TOKEN_NUMBERINT;
                                result.uvalue32 = interpret_binary_string(binary_string, length);
                                return result;
                            } break;
                            default: {
                            } break;
                        }
                    }

                    bool is_float = false;


                    bool found_one_dot = false;
                    while (((!finished()) &&
                            (source[read_cursor] == '.' && found_one_dot == false)) ||
                           (isdigit(source[read_cursor]))) {
                        if (source[read_cursor] == '.') {
                            if (!found_one_dot) {
                                found_one_dot = true;
                            }
                        }
                        read_cursor++;
                    }
                    result.string = source.substr(start_of_digit, read_cursor-start_of_digit);

                    if (found_one_dot) is_float = true;

                    if (!is_float) {
                        result.type = TOKEN_NUMBERINT;
                        result.value32 = atoi(std::string(result.string).c_str());
                    } else {
                        result.type = TOKEN_NUMBERFLOAT;
                        result.value32f = atof(std::string(result.string).c_str());
                    }

                    return result; 
                } else if (is_good_for_identifier(current)){
                    Token result      = {};
                    int symbol_start = read_cursor-1;
                    result.type       = TOKEN_SYMBOL;
                    result.col_found  = read_cursor;
                    result.line_found = line;

                    while (is_good_for_identifier(source[read_cursor]) && !finished()) {
                        read_cursor++;
                    }

                    result.string = source.substr(symbol_start, read_cursor-symbol_start);
                    return result; 
                } else {
                    assert(0 && "Bad identifier start? Should not happen");
                }
                return {};
            } break;
        }
    }
    return {};
}
