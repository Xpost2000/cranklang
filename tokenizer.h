#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstdint>

#include <string>
#include <string_view>

#define unwrap_string_view(x) x.length(), x.data()

/*
  Technically it's more useful to have tokens
  in a more granular unit, but this language is kind
  of simple so some tokens are multiple characters
*/
enum Token_Type {
    TOKEN_NONE,
    TOKEN_SYMBOL, // TODO: need table.
    // NOTE: should parse different literal types
    //   BINARY, OCTAL, HEX
    TOKEN_NUMBERINT, // for simplicity I'm only using 32bit numbers cause this is a hack project I guess
    TOKEN_NUMBERFLOAT, // for simplicity I'm only using 32bit numbers cause this is a hack project I guess
    TOKEN_STRING, // TODO: need intern table.
    TOKEN_CHARACTER,

    TOKEN_COMMENT, /* Store comments just in case. */

    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_SEMICOLON,

    TOKEN_LEFT_PARENTHESIS,
    TOKEN_RIGHT_PARENTHESIS,
    TOKEN_LEFT_CURLY_BRACE,
    TOKEN_RIGHT_CURLY_BRACE,
    TOKEN_LEFT_SQUARE_BRACE,
    TOKEN_RIGHT_SQUARE_BRACE,
    // TOKEN_LEFT_ANGLE_BRACE,
    // TOKEN_RIGHT_ANGLE_BRACE,

    TOKEN_EQUAL, // =
    TOKEN_ADDEQUAL, // +=
    TOKEN_SUBEQUAL, // -=
    TOKEN_MULEQUAL, // *=
    TOKEN_DIVEQUAL, // /=
    TOKEN_MODEQUAL, // %=

    // NOTE: I am not considering += and friends as
    // expressions per say. They are just statements in this language.

    /* MATHEMATICAL OPERATORS */
    TOKEN_ADD, // +
    TOKEN_SUB, // -
    TOKEN_MUL, // *
    TOKEN_DIV, // /
    TOKEN_MOD, // %
    // NOTE: add exponent or abs? just for fun? It is just a toy language.

    /* boolean operators*/
    TOKEN_EQUALITY, // ==
    TOKEN_NOTEQUAL, // !=
    TOKEN_AND, // &&
    TOKEN_OR, // ||
    TOKEN_NOT, // !
    TOKEN_LT, // <
    TOKEN_GT, // >
    TOKEN_LTE, // <=
    TOKEN_GTE, // >=

    /* bitwise operators */
    /* NOTE: do not have compound assignment operators for these */
    TOKEN_BITAND, // &
    TOKEN_BITOR,  // |
    TOKEN_BITNOT, // ~
    TOKEN_BITXOR, // ^

    TOKEN_COUNT,
};

static std::string_view Token_Type_string_table[] = {
    "none",
    "symbol",
    "numberint",
    "numberfloat",
    "string",
    "character",

    "comment",

    "comma",
    "dot",
    "colon",
    "semicolon",

    "leftparen",
    "rightparen",
    "leftcurly",
    "rightcurly",
    "leftsquare",
    "rightsquare",

    "equal",
    "addequal",
    "subequal",
    "mulequal",
    "divequal",
    "modequal",

    "add",
    "sub",
    "mul",
    "div",
    "mod",

    "equality",
    "notequal",
    "and",
    "or",
    "not",
    "lt",
    "gt",
    "lte",
    "gte",

    "count"
};

struct Token {
    Token() = default;
    Token(const Token&) = default;
    ~Token() = default;

    Token_Type type;

    /*
      "interpreter" is naive, and will keep all files alive until
      parsing is finished or whatever.
    */
    std::string_view string;
    int col_found;
    int line_found;

    std::string stringvalue; /* (after escape processing) only for strings because you can escape characters. */

    union {
        int32_t value32;
        float   value32f;
        char    valuechar;
    };
};

/*
  Dumb token spitter.
*/
struct Tokenizer_State {
    std::string_view source;
    int   read_cursor;
    int   line;

    Tokenizer_State(std::string_view str) : source(str), read_cursor(0), line(1) {
        
    }

    Token read_next();
    Token peek_next();

    bool  finished() {
        return read_cursor >= source.size();
    }

    char next_character() {
        if (finished()) return 0;
        return source[read_cursor++];
    }

    char peek_character() {
        char result =  next_character();
        read_cursor--;
        return result;
    }
};

#endif
