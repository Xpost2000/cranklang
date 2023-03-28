/*
  A small programming language interpreter/transpiler

  NOTE: have to refactor like all of this later...
  because it's not object oriented.

  Expressions are not typechecked properly.

  Code is kind of messy because it's a "hack" project.

  I'll try to clean it up and at the very least have some decent error
  propagation / reporting.
  
  NOTE: in transpiler mode, I don't have to evaluate the expressions... I just need
  to make sure they evaluate to the right type.

  Most of this code is "backend"/"compiler" logic.

  The interpreter logic will be based off evaluating the tree later.

  One of them quick hack projects,
  I do with a bit of time.

  A minimally useful language?

  - Array Programming
  - Lambdas/local functions
  - Structure Extension
  - Basic Generics?
  - Out of order implementation


  - [X] Tokenizer done!

  TODO: Proper error handling takes a lot of code, and I don't have that
  much time so I've kinda stopped doing it at some point just to get sutff done
  faster.

  // BOOLEAN!
    - proper type checking on values.
    - Keyword restriction?
    -  need to handle symbol lookup
    -  variable multiple variable redeclarations. (shadowing should be okay.)
    - Parsing expression trees.
    - Function Declaration
*/
#include "tokenizer.h"
#include <cstdio>

#include "utilities.cc"
#include "error.h"

#include <vector>
#include <unordered_map>

enum Crank_Declaration_Type {
    DECL_OBJECT, // Functions & Variables. I want them to work as part of the same type system.
    DECL_TYPE, // typedef or record
};

// simple type system
enum Crank_Types {
    TYPE_NUMERIC, // unresolved numeric type. Will be converted as needed.
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRINGLITERAL,
    TYPE_RECORD,
    TYPE_VOID,
    TYPE_RENAME, // typedef, this doesn't really have much significance, so I might remove this.
    TYPE_COUNT
};

// some forward decls. This is going to be messy.
struct Crank_Type;
struct Crank_Object_Literal;

struct Crank_Object_Decl_Base {
    Crank_Type* object_type;
    std::vector<int> array_dimensions; // empty is not an array. -1 means don't care. might be flexible.
    std::string name;
};

enum Crank_Value_Type {
    VALUE_TYPE_LITERAL, // Object literal... Number, String literal, Array Literal?
    VALUE_TYPE_SYMBOL,  // Symbol name to look up. Hopefully the right type.
};

enum Crank_Object_Literal_Decl_Type {
    OBJECT_LITERAL_DECL_ORDERED,  // { 2, 4, "asdf", 4 }
    OBJECT_LITERAL_DECL_NAMED,    // { x: 2, y: 4, string: asdf, f: 14 }
};


// Might change to object oriented.
// NOTE: fat struct.
struct Crank_Value {
    int value_type; // This is just the obligatory "parse" information
    Crank_Type* type; // This is the actual discriminator for the value type.

    // TODO:
    // used for literals only. Type for symbol references,
    // will be looked up according to context

    // if it's a symbol, don't worry about the type for now.
    union {
        int   int_value;
        float float_value;
        Crank_Object_Literal* literal_value;
    };
    std::string string_value; // should be interned.

    // used for arrays.
    std::vector<Crank_Value> array_elements;

    // used for functions
    bool is_function_call;
    std::vector<Crank_Value> call_parameters;
    // TODO: add list of statements.
    // functions are theoretically values in Crank.

    // look up symbol name for values.
    std::string symbol_name;
};

struct Crank_Object_Named_Value {
    std::string name;
    Crank_Value value;
};
// Can only be one or another, cannot be both.
struct Crank_Object_Literal {
    int type;
    union {
        std::vector<Crank_Object_Named_Value> named_values;
        std::vector<Crank_Value> values;
    };
};

// NOTE: declarations are mainly toplevel objects.
// Although we do have those types inside of function bodies or statements.
struct Crank_Declaration : public Crank_Object_Decl_Base {
    int decl_type;
    // int export; // TODO: module system!
    /*
      Will be created if it's a DECL_TYPE, based off parsing.
      (or if it's a typedef it'll just match.)

      Otherwise it's just the type of the object.
     */

    // Used only for DECL_OBJECT
    Crank_Value value;
};

struct Crank_Record_Member : public Crank_Object_Decl_Base {
};

struct Crank_Type {
    int type;
    Crank_Type* rename_of;
    std::string name;
    // TODO: for now shove this in here until I figure out how this array thing works.
    std::vector<int> array_dimensions; // empty is not an array. -1 means don't care. might be flexible.
    std::vector<Crank_Record_Member> members;
};

bool crank_type_match(Crank_Type* a, Crank_Type* b) {
    // Since I intern Crank_Types this is always okay.
    if (a == b) {
        return true;
    }

    // NOTE: Typedef checking. We'll follow back all the references
    {
        // I don't believe I need to check this twice
        // but this is just in-case.
        Crank_Type* typedef_a = a;
        while (typedef_a) {
            if (typedef_a == b) {
                return true;
            }
            typedef_a = typedef_a->rename_of;
        }
        Crank_Type* typedef_b = b;
        while (typedef_b) {
            if (typedef_b == a) {
                return true;
            }
            typedef_b = typedef_b->rename_of;
        }
    }

    if (a->array_dimensions.size() > 0 && b->array_dimensions.size() > 0) {
    }

    return false;
}

// globally registered types TODO: make this per module. I'm just doing this to make resolution easy until I figure out what I'm doing.
std::vector<Crank_Type*> global_type_table; // too many allocations, but I don't want to deal with pointer fix up right now.
Crank_Type* register_new_type(std::string_view name, int type, std::vector<int> array_dimensions={}) {
    global_type_table.push_back(new Crank_Type);
    Crank_Type* result = global_type_table.back();
    result->type = type;
    result->name = std::string(name);
    result->array_dimensions = array_dimensions;
    return result;
}
Crank_Type* lookup_type(std::string_view name, std::vector<int> array_dimensions={}) {
    // NOTE: when looking up an array variant
    //       type.
    //
    //
    // I do consider them "separate" types (as in they produce new
    // type entries, as I'm not sure of how else to handle them),
    //
    // however they are special cased.
    // NOTE: array types will be created dynamically unlike normal types.
    //
    // Array dimensions depend on their base type to exist,
    // so we will look up if the same name without the array specifier
    // exists first.

    // Do a more thorough type check.
    if (array_dimensions.size() > 0) {
        if (lookup_type(name)) {
            Crank_Type* result = nullptr;
            for (auto type : global_type_table) {
                if (type->name == name) {
                    result = type;
                    if (type->array_dimensions.size() == array_dimensions.size()) {
                        for (int i = 0; i < type->array_dimensions.size(); ++i) {
                            if (type->array_dimensions[i] != array_dimensions[i]) {
                                result = nullptr;
                            }
                        }
                    } else {
                        result = nullptr;
                    }
                }
            }

            if (!result) {
                // register the arrayed version of the type.
                // it will be cached. This is not the best way to do the type
                // system. However arrays are distinct types which is a sane thing
                // to do imo.

                // NOTE:
                // Arrays share the same "base value_type" as their
                // scalar counterpart.
                result = register_new_type(name, lookup_type(name)->type, array_dimensions);
            }

            return result;
        } else {
            return nullptr;
        }
    } else {
        for (auto type : global_type_table) {
            if (type->name == name) {
                return type;
            }
        }
    }

    return nullptr;
}

struct Crank_Module {
    /* general purpose declaration list. */
    /* should be separated into more specific types to make things easier to look at. */
    std::string module_name; // usually just the file name. Can be overriden?
    std::vector<Crank_Module*> imports;
    std::vector<Crank_Declaration> decls;
};

struct Crank_Type_Declaration { // NOTE: for semantic analysis. Not doing type system things here!
    std::string name;
    std::vector<int> array_dimensions;
};

// -1 is bad
// TODO make this an error type
// it's like a weird stream api.
int read_array_specifier(Tokenizer_State& tokenizer, int* out) {
    assert(out && "need output pointer.");

    auto first = tokenizer.peek_next();
    if (first.type == TOKEN_LEFT_SQUARE_BRACE) {
        // do parsing
        tokenizer.read_next();
        auto second = tokenizer.read_next();
        if (second.type == TOKEN_NUMBERINT) {
            *out = second.value32;
            auto third = tokenizer.read_next();
            if (third.type != TOKEN_RIGHT_SQUARE_BRACE) {
                printf("need to close speciifer!\n");
                return -1;
            } else {
                return 1;
            }
        } else if (second.type == TOKEN_RIGHT_SQUARE_BRACE) {
            printf("Identified flexibly sized array member.\n");
            return 1;
        } else {
            printf("Should not be non-int type!\n");
            return -1;
        }
    }

    return 0;
}

Error<Crank_Type_Declaration> read_type_declaration(Tokenizer_State& tokenizer) {
    auto name = tokenizer.read_next();
    if (name.type != TOKEN_SYMBOL) return Error<Crank_Type_Declaration>::fail("Bad type declaration.");

    std::vector<int> array_dimensions;

    while (true) {
        int current_dimension = -1;
        int read_result = read_array_specifier(tokenizer, &current_dimension);
        if (read_result == 1) {
            array_dimensions.push_back(current_dimension);
        } else if (read_result == 0) {
            break;
        } else if (read_result == -1)  {
            return Error<Crank_Type_Declaration>::fail("Fail to read array specifier.");
        }
    }

    Crank_Type_Declaration result;

    result.name = name.string;
    result.array_dimensions = array_dimensions;

    return Error<Crank_Type_Declaration>::okay(result);
}

struct Inline_Decl : public Crank_Object_Decl_Base {
    bool has_value;
    Crank_Value value;
};

/* A mathematical expression or boolean expression */
// NOTE: not comma.
enum Crank_Expression_Type {
    EXPRESSION_VALUE,
    EXPRESSION_UNARY,
    EXPRESSION_BINARY,
};

enum Crank_Expression_Operator {
    OPERATOR_NOT, // unary
    OPERATOR_NEGATE,

    // binary...
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
    OPERATOR_MOD,

    /* NOTE: when evaluating make sure that lvalue is a symbol! */
    OPERATOR_EQUAL,
    OPERATOR_ADDEQUAL,
    OPERATOR_SUBEQUAL,
    OPERATOR_MULEQUAL,
    OPERATOR_DIVEQUAL,
    OPERATOR_MODEQUAL,

    OPERATOR_NOT_EQUALITY,
    OPERATOR_EQUALITY,
    OPERATOR_LT,
    OPERATOR_GT,
    OPERATOR_LTE,
    OPERATOR_GTE,

    // that's about it.
    OPERATOR_AND,
    OPERATOR_OR,
};

const char* Crank_Expression_Operator_string_table[] = {
    "(not)",
    "(negate)",

    "(add)",
    "(sub)",
    "(mul)",
    "(div)",
    "(mod)",

    "(=)",
    "(+=)",
    "(-=)",
    "(*=)",
    "(/=)",
    "(%=)",

    "(ne)",
    "(eq)",
    "(lt)",
    "(gt)",
    "(lte)",
    "(gte)",

    "(and)",
    "(or)",

    "(count)"
};

// TODO: separate this into a file somewhere else.
// TODO: rewrite it as a set of objects.
struct Crank_Expression {
    int type;
    int operation;

    union {
        struct {
            Crank_Expression* value; // Unary
        } unary;

        struct {
            Crank_Expression* first;
            Crank_Expression* second;
        } binary;
    };
    // NOTE: not part of the union because it would not be trivial lol
    Crank_Value value;
};

int token_to_operation(Token t) {
    switch (t.type) {
        case TOKEN_NOT: return OPERATOR_NOT;
        case TOKEN_SUB: return OPERATOR_SUB;
        case TOKEN_ADD: return OPERATOR_ADD;
        case TOKEN_MUL: return OPERATOR_MUL;
        case TOKEN_DIV: return OPERATOR_DIV;
        case TOKEN_MOD: return OPERATOR_MOD;

        case TOKEN_EQUAL:    return OPERATOR_EQUAL;
        case TOKEN_ADDEQUAL: return OPERATOR_ADDEQUAL;
        case TOKEN_SUBEQUAL: return OPERATOR_SUBEQUAL;
        case TOKEN_MULEQUAL: return OPERATOR_MULEQUAL;
        case TOKEN_DIVEQUAL: return OPERATOR_DIVEQUAL;
        case TOKEN_MODEQUAL: return OPERATOR_MODEQUAL;

        case TOKEN_EQUALITY: return OPERATOR_EQUALITY;
        case TOKEN_NOTEQUAL: return OPERATOR_NOT_EQUALITY;

        case TOKEN_AND: return OPERATOR_AND;
        case TOKEN_OR: return OPERATOR_OR;

        case TOKEN_LT: return OPERATOR_LT;
        case TOKEN_GT: return OPERATOR_GT;

        case TOKEN_LTE: return OPERATOR_LTE;
        case TOKEN_GTE: return OPERATOR_GTE;
        default: {
            assert(0 && "Invalid token type.");
            return -1;
        } break;
    }
}

Crank_Expression* binary_expression(Crank_Expression* left, Crank_Expression* right, int operation) {
    Crank_Expression* result = new Crank_Expression;
    result->type = EXPRESSION_BINARY;
    result->operation = operation;
    result->binary.first = left;
    result->binary.second = right;
    return result;
}
Crank_Expression* unary_expression(Crank_Expression* value, int operation) {
    Crank_Expression* result = new Crank_Expression;
    result->type = EXPRESSION_UNARY;
    result->operation = operation;
    result->unary.value = value;
    return result;
}
Crank_Expression* value_expression(Crank_Value value) {
    Crank_Expression* result = new Crank_Expression;
    result->type = EXPRESSION_VALUE;
    result->value = value;
    return result;
}

Inline_Decl read_inline_declaration(Tokenizer_State& tokenizer);
Error<Crank_Value> read_value(Tokenizer_State& tokenizer);

// builds an expression tree.
// without type checking.
// although type checking shouldn't be impossible.
// recursive descent parsing.

// Type checking a value will be done after parsing.
// This is just meant to produce a tree.

// This is just a textbook recursive descent algorithm
Crank_Expression* parse_expression(Tokenizer_State& tokenizer);

// =, +=, -=, ...
Crank_Expression* parse_assignment_or_compound(Tokenizer_State& tokenizer);
// ==, !=
Crank_Expression* parse_equality(Tokenizer_State& tokenizer);
// <= >= < >
Crank_Expression* parse_comparison(Tokenizer_State& tokenizer);
// +/-
Crank_Expression* parse_term(Tokenizer_State& tokenizer);
// */div or mod
Crank_Expression* parse_factor(Tokenizer_State& tokenizer);
// - or !
Crank_Expression* parse_unary(Tokenizer_State& tokenizer);
Crank_Expression* parse_value(Tokenizer_State& tokenizer);

Crank_Expression* parse_value(Tokenizer_State& tokenizer) {
    if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
        tokenizer.read_next();
        auto expression = parse_expression(tokenizer);
        assert(tokenizer.peek_next().type == TOKEN_RIGHT_PARENTHESIS && "Incorrectly terminated expression!");
        tokenizer.read_next();
        return expression;
    } else {
        auto value_read = read_value(tokenizer);
        assert(value_read.good && "Failed to read value?");
        return value_expression(value_read.value);
    }
}

Crank_Expression* parse_unary(Tokenizer_State& tokenizer) {
    auto peek_next = tokenizer.peek_next();
    if (peek_next.type == TOKEN_NOT || peek_next.type == TOKEN_SUB) {
        printf("Unary found!\n");
        auto next = tokenizer.read_next(); 
        int operation = token_to_operation(next);

        // to course-correct.
        if (operation == OPERATOR_SUB) {
            operation = OPERATOR_NEGATE;
        }

        assert(operation != -1 && "something went wrong here.");
        return unary_expression(parse_value(tokenizer), operation);
    }

    return parse_value(tokenizer);
}

Crank_Expression* parse_factor(Tokenizer_State& tokenizer) {
    Crank_Expression* result = parse_unary(tokenizer);

    while (
        tokenizer.peek_next().type == TOKEN_MUL ||
        tokenizer.peek_next().type == TOKEN_DIV ||
        tokenizer.peek_next().type == TOKEN_MOD
    ) {
        auto operator_token = tokenizer.read_next();
        int operation = token_to_operation(operator_token);

        auto right_value = parse_unary(tokenizer);
        result = binary_expression(result, right_value, operation);
    }

    return result;
}

Crank_Expression* parse_term(Tokenizer_State& tokenizer) {
    Crank_Expression* result = parse_factor(tokenizer);

    while (
        tokenizer.peek_next().type == TOKEN_ADD ||
        tokenizer.peek_next().type == TOKEN_SUB
    ) {
        auto operator_token = tokenizer.read_next();
        int operation = token_to_operation(operator_token);

        auto right_value = parse_factor(tokenizer);
        result = binary_expression(result, right_value, operation);
    }

    return result;
}

Crank_Expression* parse_comparison(Tokenizer_State& tokenizer) {
    Crank_Expression* result = parse_term(tokenizer);

    while (
        tokenizer.peek_next().type == TOKEN_GT ||
        tokenizer.peek_next().type == TOKEN_LT ||
        tokenizer.peek_next().type == TOKEN_GTE ||
        tokenizer.peek_next().type == TOKEN_LTE
    ) {
        auto operator_token = tokenizer.read_next();
        int operation = token_to_operation(operator_token);

        auto right_value = parse_term(tokenizer);
        result = binary_expression(result, right_value, operation);
    }

    return result;
}

Crank_Expression* parse_equality(Tokenizer_State& tokenizer) {
    Crank_Expression* result = parse_comparison(tokenizer);

    while (tokenizer.peek_next().type == TOKEN_EQUALITY ||
           tokenizer.peek_next().type == TOKEN_NOTEQUAL) {
        auto operator_token = tokenizer.read_next();
        int operation = token_to_operation(operator_token);

        auto right_value = parse_comparison(tokenizer);
        result = binary_expression(result, right_value, operation);
    }

    return result;
}

// NOTE: right associative
Crank_Expression* parse_assignment_or_compound(Tokenizer_State& tokenizer) {
    Crank_Expression* result = parse_equality(tokenizer);

    while (
        tokenizer.peek_next().type == TOKEN_EQUAL   ||
        tokenizer.peek_next().type == TOKEN_ADDEQUAL||
        tokenizer.peek_next().type == TOKEN_SUBEQUAL||
        tokenizer.peek_next().type == TOKEN_MULEQUAL||
        tokenizer.peek_next().type == TOKEN_DIVEQUAL||
        tokenizer.peek_next().type == TOKEN_MODEQUAL
    ) {
        auto operator_token = tokenizer.read_next();
        int operation = token_to_operation(operator_token);
        auto right_value = parse_assignment_or_compound(tokenizer);
        result = binary_expression(result, right_value, operation);
    }

    return result;
}

Crank_Expression* parse_expression(Tokenizer_State& tokenizer) {
    return parse_assignment_or_compound(tokenizer);
}


void _debug_print_crank_value(Crank_Value value) {
    switch (value.value_type) {
        case VALUE_TYPE_LITERAL: {
            // nested objects like arrays are not going to be fun.
            if (value.array_elements.size()) {
                printf("(array (unprintable-for-now)) ");
            } else {
                if (value.type->type == TYPE_INTEGER) {
                    printf("(int %d) ", value.int_value);
                } else if (value.type->type == TYPE_FLOAT) {
                    printf("(float %3.3f) ", value.float_value);
                } else if (value.type->type == TYPE_BOOLEAN) {
                    printf("(bool %s) ", (value.int_value == 1) ? "TRUE" : "FALSE");
                } else if (value.type->type == TYPE_STRINGLITERAL) {
                    printf("(strlit \"%s\") ", value.string_value.c_str());
                } else {
                    printf("(unprintable) ");
                }
            }
        } break;
        case VALUE_TYPE_SYMBOL: {
            if (value.is_function_call) {
                printf("(funcall %s [%d params]) ", value.symbol_name.c_str(), value.call_parameters.size());
            } else {
                printf("(sym %s) ", value.symbol_name.c_str());
            }
        } break;
    }
}
// NOTE: remove later
void _debug_print_expression_tree(Crank_Expression* root) {
    if (root)
    switch (root->type) {
        case EXPRESSION_VALUE: {
            _debug_print_crank_value(root->value);
        } break;
        case EXPRESSION_UNARY: {
            printf("(%s ", Crank_Expression_Operator_string_table[root->operation]);
            if (root->unary.value) _debug_print_expression_tree(root->unary.value);
            printf(") ");
        } break;
        case EXPRESSION_BINARY: {
            if (root->binary.first)
            printf("(%s ", Crank_Expression_Operator_string_table[root->operation]);
            _debug_print_expression_tree(root->binary.first);
            if (root->binary.second) _debug_print_expression_tree(root->binary.second);
            printf(") ");
        } break;
    }
}

Error<Crank_Value> read_value(Tokenizer_State& tokenizer) {
    auto first = tokenizer.read_next();
    Crank_Value value;
    value.value_type = VALUE_TYPE_LITERAL;

    switch(first.type) {
        case TOKEN_STRING: {
            printf("Found string literal.\n");
            value.type = lookup_type("strlit");
            value.string_value = first.stringvalue;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_NUMBERINT: {
            printf("Found number.\n");
            value.type = lookup_type("int");
            value.int_value = first.value32;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_NUMBERFLOAT: {
            printf("Found float number.\n");
            value.type = lookup_type("float");
            value.float_value = first.value32f;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_SYMBOL: {
            printf("Found symbol.\n");
            auto next = tokenizer.peek_next();
            value.value_type = VALUE_TYPE_SYMBOL;
            if (next.type == TOKEN_LEFT_CURLY_BRACE) { // NOTE: Does this really have to be part of the syntax?
                tokenizer.read_next();
                // object literal
                value.value_type = VALUE_TYPE_LITERAL;
                value.type = lookup_type(first.string);
                // TODO
                {
                    /* here we go! */
                    // keep reading declarations.
                    printf("HI THIS IS NOT DONE!");
                    assert(0 && "HI I'M NOT DONE");
                }
                assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
            } else {
                // symbol
                value.symbol_name = first.string;
                if (next.type == TOKEN_LEFT_PARENTHESIS) {
                    tokenizer.read_next(); 

                    value.is_function_call = true;
                    while (tokenizer.peek_next().type != TOKEN_RIGHT_PARENTHESIS) {
                        auto new_value = read_value(tokenizer);
                        assert(new_value.good && "Bad function param passing");
                        value.call_parameters.push_back(new_value.value);

                        if (tokenizer.peek_next().type == TOKEN_COMMA) {
                            tokenizer.read_next();
                        } else if (tokenizer.peek_next().type == TOKEN_RIGHT_PARENTHESIS) {
                            break;
                        } else {
                            assert(0 && "Bad call params list! Needs comma separated parameters");
                        }
                    }
                    tokenizer.read_next();
                }
                return Error<Crank_Value>::okay(value);
            }
        } break;
            // array literal!
            // NOTE: not type checked here.
            // we'll type check values after they're read.
        case TOKEN_LEFT_SQUARE_BRACE: {
            // literal type.
            while (tokenizer.peek_next().type != TOKEN_RIGHT_SQUARE_BRACE) {
                auto new_value = read_value(tokenizer);
                assert(new_value.good && "Bad array literal parsing?");
                value.array_elements.push_back(new_value.value);

                if (tokenizer.peek_next().type == TOKEN_COMMA) {
                    tokenizer.read_next();
                } else if (tokenizer.peek_next().type == TOKEN_RIGHT_SQUARE_BRACE) {
                    break;
                } else {
                    assert(0 && "Bad array literal! Need comma separated values!");
                }
            }
            tokenizer.read_next();
            return Error<Crank_Value>::okay(value);
        } break;
    }

    return Error<Crank_Value>::fail("bad value");
}

// TODO: Does not type check yet.
bool do_array_typecheck(Crank_Type* type,
                        std::vector<int> array_dimensions,
                        std::vector<Crank_Value> array_elements) {
    auto expected_element_count = array_dimensions[0];

    // -1 means flexible. So it will always pass that check.
    if (expected_element_count == -1 || array_elements.size() == expected_element_count) {
        if (array_dimensions.size() > 1) { // multi dimensional array
            array_dimensions.erase(array_dimensions.begin());
            // This is n^2 unfortunately because we have to check everything...
            for (auto& element : array_elements) {
                if (!do_array_typecheck(type, array_dimensions, element.array_elements)) {
                    printf("Failed array typecheck! Not matching element count!\n");
                    return false;
                }
            }
            return true;
        } else {
            return true;
        }
    }

    return false;
}

// Any form that resembles a variable declaration.
Inline_Decl read_inline_declaration(Tokenizer_State& tokenizer) {
    Inline_Decl result;
    result.has_value = false;

    auto name = tokenizer.read_next();
    assert(name.type == TOKEN_SYMBOL);
    auto colon = tokenizer.read_next();
    assert(colon.type == TOKEN_COLON);

    auto type_entry = read_type_declaration(tokenizer);
    assert(type_entry.good && "Bad type entry?");
    auto type = lookup_type(type_entry.value.name, type_entry.value.array_dimensions);
    assert(type && "Type not found! Cannot resolve!");

    // TODO: array specifier.

    result.array_dimensions = type_entry.value.array_dimensions;

    if (tokenizer.peek_next().type == TOKEN_EQUAL) {
        tokenizer.read_next();
        auto value = read_value(tokenizer);
        assert(value.good);
        result.has_value = true;
        result.value     = value;

        // assert that the evaluated type should match the type we
        // found.

        // NOTE: arrays have to be typed here. We'll enforce a type check on all
        // elements which is kind of slow since I should've tested it while parsing?
        // however the code is not organized to allow this for now.
        if (result.value.array_elements.size() > 0) {
            printf("Array type checking\n");
            result.value.type = type;

            std::vector<int> array_dimensions = result.value.type->array_dimensions;
            assert(do_array_typecheck(type, array_dimensions, value.value.array_elements) && "check the message.");
        }

        assert(value.value.type && "Value does not have a type for some reason?");

        // This is not being done yet.

        // NOTE: for arrays this is loser. As long as long as it can
        // be cast to that thing.
    }

    assert(tokenizer.read_next().type == TOKEN_SEMICOLON);
    return result;
}

bool read_record_definition(Crank_Type* type, Tokenizer_State& tokenizer) {
    assert(type->type == TYPE_RECORD && "wtf");
    printf("Trying to read decl\n");
    while (tokenizer.peek_next().type == TOKEN_SYMBOL) {
        auto new_member_decl = read_inline_declaration(tokenizer);
        // TODO: check for duplicates!
        if (new_member_decl.has_value) {
            printf("Sorry! No default value yet!\n");
        }
        Crank_Record_Member member;
        member.name             = new_member_decl.name;
        member.array_dimensions = new_member_decl.array_dimensions;
        member.object_type      = new_member_decl.object_type;

        type->members.push_back(member);
    }
    printf("Finished reading decl! (%d members?)\n", type->members.size());
    assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
    return true;
}

Error<Crank_Declaration> parse_typedef(Crank_Module& module, Tokenizer_State& tokenizer) {
    auto name = tokenizer.read_next();
    if (name.type != TOKEN_SYMBOL) return Error<Crank_Declaration>::fail("Typedef name is not a symbol!");
    auto colon_separator = tokenizer.read_next();
    if (colon_separator.type != TOKEN_COLON) return Error<Crank_Declaration>::fail("Need separator!");
    auto determiner = tokenizer.peek_next();

    if (determiner.type == TOKEN_SYMBOL) {
        // typedef of existing. Rename!
        auto type_entry = read_type_declaration(tokenizer);

        if (!type_entry.good) return Error<Crank_Declaration>::fail(type_entry.message);

        Crank_Declaration typedecl;
        typedecl.decl_type = DECL_OBJECT;
        typedecl.object_type = lookup_type(determiner.string);
        typedecl.name = name.string;
        typedecl.array_dimensions = type_entry.value.array_dimensions;

        // NOTE: if we rename based off an array type
        // there's no need to list the array dimensions.
        auto new_type = register_new_type(typedecl.name, TYPE_RENAME);
        new_type->rename_of = lookup_type(determiner.string, typedecl.array_dimensions);
        assert(new_type->rename_of && "Error! Typedefed type does not exist!");

        if (!typedecl.object_type) {
            // Error. Cannot resolve currently.
            // TODO: I would resolve this later but I don't have the resources to do so right now!
            return Error<Crank_Declaration>::fail("Cannot resolve type! Not known yet?");
        }
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON);
        printf("read typedef rename\n");

        return Error<Crank_Declaration>::okay(typedecl);
    } else if (determiner.type == TOKEN_LEFT_CURLY_BRACE) {
        // record type.
        tokenizer.read_next();

        Crank_Declaration typedecl;
        typedecl.decl_type = DECL_OBJECT;
        typedecl.object_type = register_new_type(name.string, TYPE_RECORD);

        if (read_record_definition(typedecl.object_type, tokenizer)) {
            printf("Read new record definition\n");
            return Error<Crank_Declaration>::okay(typedecl);
        } else {
            return Error<Crank_Declaration>::fail("Failed to read record definition");
        }
        // typedecl.array_dimensions;
        typedecl.name = name.string;
    }


    return Error<Crank_Declaration>::fail("Failed to parse typedef.");
}

Error<Crank_Declaration> parse_variable_decl(Crank_Module& module, Tokenizer_State& tokenizer) {
    auto inline_decl = read_inline_declaration(tokenizer);
    Crank_Declaration decl;
    decl.decl_type = DECL_OBJECT;
    decl.object_type = inline_decl.object_type;
    decl.array_dimensions = inline_decl.array_dimensions;
    decl.name = inline_decl.name;
    // do something about 'has value'?
    decl.value = inline_decl.value;
    return Error<Crank_Declaration>::okay(decl);
}

Error<Crank_Module> load_module_from_source(std::string module_name, std::string_view source_code) {
    Crank_Module module;
    module.module_name = module_name;

    Tokenizer_State tokenizer(source_code);

    while (!tokenizer.finished()) {
        auto first_token = tokenizer.peek_next();

        /*
          Reading the top level.
         */
        switch (first_token.type) {
            case TOKEN_COMMENT: tokenizer.read_next(); continue; // ignore
            case TOKEN_SYMBOL: {
                if (first_token.string == "typedef") {
                    tokenizer.read_next();
                    auto new_typedef = parse_typedef(module, tokenizer);
                    if (!new_typedef.good) {
                        printf("%s\n", new_typedef.message);
                    } else 
                        module.decls.push_back(new_typedef);
                } else {
                    // TODO: Check for duplicates (IN THE SAME SCOPE!)
                    auto new_decl = parse_variable_decl(module, tokenizer);
                    if (!new_decl.good) {
                        printf("%s\n", new_decl.message);
                    } else 
                        module.decls.push_back(new_decl);
                }

            } break;
            default: {
                /* TODO better error message? */
                printf("What did I read?: %.*s\n", unwrap_string_view(Token_Type_string_table[first_token.type]));
                return Error<Crank_Module>::fail(
                    "Not a valid declaration start!"
                );
            } break;
        }
    }

    return Error<Crank_Module>::okay(module);
}
void register_default_types() {
    register_new_type("int",   TYPE_INTEGER);
    register_new_type("float", TYPE_FLOAT);
    register_new_type("bool",  TYPE_BOOLEAN);
    register_new_type("strlit",TYPE_STRINGLITERAL);
    register_new_type("void",  TYPE_VOID);
}

int main(int argc, char** argv){
    register_default_types();

    // Tokenizer_State tokenizer("2 == (4 != 5)");
    // char* test_parse = "3 * ([1, 2, 3] + [4, 5, 7])";
    char* test_parse = "test_function() * 5 + test_function4(4, 4, 6)";
    printf("Parsing: %s\n", test_parse);
    Tokenizer_State tokenizer(test_parse);
    auto t = parse_expression(tokenizer);
    _debug_print_expression_tree(t);
    printf("hi, did you crash\n");
    
    // File_Buffer test_to_tokenize = File_Buffer("simplevars.crank");
    // auto e = load_module_from_source("simplevars", test_to_tokenize.data);
    // printf("%s\n", e.message);
    return 0;
}
