/*
  NOTE: It appears I am probably letting go of the interpreter idea, and just focusing on the transpiler.
  To the best of it's ability, crank is still checking stuff so it is still an actual language.

  A small programming language transpiler. It's not really efficient
  it's just to figure out how to write a language by any means
  necessary. So there's an allocation festival thanks to all the
  std::string I use.

  4/24/2023: Hmm, after a bit of reading I believe it is impertinent that I restructure
  the "compiler" to only produce an AST that is unchecked...

  NOTE: I'm not deleting anything here because
  the OS will reclaim all memory, and it's only running until
  the compiler finishes. It's not real time so that's fine.

  OPTIONAL/TODO: I would like better error reporting logic, but this isn't really about
  that (IE: the intent isn't to make a sane language to use, it's just "compiler" practice)


  CODEGEN TODO:
  Object literals are not considered because they were not there when I hacked a c++ code generator.

  TODO: 
  - Reserve key words
  - Replace using Value to Expression in a lot of places.
  - Replace most instances of std::string
  There's too much copying that I'm not super comfortable with,
  but that's part of the clean up code.

  NOTE: A lot of thing in the "compiler" are not
  typechecked as of now! I am checking these functional changes
  in but invalid types are still allowed
  - Function decls will definitely not be typechecked yet
  - Typedecls are mostly okay
  - Expressions are not properly type checked!
  - Basic to C++ compiler (just compile types in I suppose)

  NOTE: have to refactor like all of this later...
  because it's not object oriented in the traditional sense.

  Code is kind of messy because it's a "hack" project.

  I'll try to clean it up and at the very least have some decent error
  propagation / reporting.
  
  NOTE: in transpiler mode, I don't have to evaluate the expressions... I just need
  to make sure they evaluate to the right type.

  Most of this code is "backend"/"compiler" logic.

  A minimally useful language?

  I need pointers

  Wishlist, I need this to be able to do as much as C can first:

  - Array Programming
  - Lambdas/local functions
  - Structure Extension
  struct E {
  int c; 
  };
  struct Z {
  int a;
  int b;
  };
  struct XY {
  using Z;
  using E;
  };

  This will be interesting to compile into C since I want this to allow for polymorphism in a weird way.
  - Out of order implementation


  - [X] Tokenizer done!

  TODO: Proper error handling takes a lot of code, and I don't have that
  much time so I've kinda stopped doing it at some point just to get sutff done
  faster.
*/
#include "crank-cpp-runtime/crank_preamble.h"
#include "tokenizer.h"
#include "utilities.cc"
#include "error.h"

enum Crank_Declaration_Type {
    DECL_OBJECT, // Functions & Variables. I want them to work as part of the same type system.
    DECL_TYPE, // typedef or record
};

// simple type system
enum Crank_Types {
    TYPE_NUMERIC, // unresolved numeric type. Will be converted as needed. 
    TYPE_BOOLEAN, // NOT IMPLEMENTED?
    TYPE_INTEGER8,
    TYPE_INTEGER16,
    TYPE_INTEGER32,
    TYPE_INTEGER64,
    TYPE_UNSIGNEDINTEGER8,
    TYPE_UNSIGNEDINTEGER16,
    TYPE_UNSIGNEDINTEGER32,
    TYPE_UNSIGNEDINTEGER64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRINGLITERAL,
    TYPE_RECORD, // NOT FULLY IMPLEMENTED
    TYPE_VOID, // NOT IMPLEMENTED
    TYPE_RENAME, // typedef, this doesn't really have much significance, so I might remove this.
    TYPE_CHAR = TYPE_INTEGER8,
    TYPE_COUNT
};

// some forward decls. This is going to be messy.
struct Crank_Type;
struct Crank_Object_Literal;

struct Crank_Object_Decl_Base {
    Crank_Type* object_type;

    //... hmm when do I use this?
    std::vector<int> array_dimensions; // empty is not an array. -1 means don't care. might be flexible.

    std::string name;
    bool has_value;
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

struct Crank_Statement;
struct Crank_Expression;
struct Crank_Value {
    int value_type; // This is just the obligatory "parse" information
    Crank_Type* type; // This is the actual discriminator for the value type.

    // TODO:
    // used for literals only. Type for symbol references,
    // will be looked up according to context

    // if it's a symbol, don't worry about the type for now.
    union {
        uint64_t   uint_value;
        int64_t   int_value;
        double float_value;
        Crank_Object_Literal* literal_value;
    };
    std::string string_value; // should be interned.
    // std::string unescaped_string_value; // I should escape when evaling.

    // used for arrays. (initializers)
    std::vector<Crank_Expression*> array_elements = {};

    // used for functions
    bool is_function_call = false;
    std::vector<Crank_Expression*> call_parameters = {};
    Crank_Statement* body = nullptr;

    // TODO: add list of statements.
    // functions are theoretically values in Crank.

    // look up symbol name for values.
    std::string symbol_name;
};

struct Crank_Object_Named_Value {
    std::string name;
    Crank_Expression* expression = nullptr;
};

// Can only be one or another, cannot be both.
struct Crank_Object_Literal {
    int type;
    std::vector<Crank_Object_Named_Value> named_values;
    std::vector<Crank_Expression*> expressions;
};

// NOTE: declarations are mainly toplevel objects.
// Although we do have those types inside of function bodies or statements.
struct Crank_External_Information {
    std::string linkage_name = "";
};

struct Crank_Declaration : public Crank_Object_Decl_Base {
    int decl_type;
    bool is_externally_defined = false; // any declaration followed by extern
    Crank_External_Information extern_definition;
    // int export; // TODO: module system!
    /*
      Will be created if it's a DECL_TYPE, based off parsing.
      (or if it's a typedef it'll just match.)

      Otherwise it's just the type of the object.
    */

    // Used only for DECL_OBJECT
    Crank_Expression* expression = nullptr;
};

// mostly needed for unit testing stuff...
Crank_Declaration make_uninitialized_object_decl(Crank_Type* type, std::string name, std::vector<int> array_dimensions={}) {
    Crank_Declaration result = {};
    result.decl_type = DECL_OBJECT;
    result.object_type = type;
    result.name = name;
    result.array_dimensions = array_dimensions;
    result.has_value = false;
    return result;
}

struct Crank_Type {
    /*
     * base type of object.
     *
     * arrays and pointers are not their own types and are considered
     * "derivative" types.
     */
    int type;
    Crank_Type* rename_of = nullptr;
    std::string name;
    // TODO: for now shove this in here until I figure out how this array thing works.
    std::vector<int> array_dimensions; // empty is not an array. -1 means don't care. might be flexible.

    int pointer_depth = 0;
    bool is_function = false;
    std::vector<Crank_Declaration> call_parameters;
    std::vector<Crank_Declaration> members;
};


// TODO:
// - Check array dimension matching
// - Check function matching!

Crank_Type* follow_typedef_chain(Crank_Type* type) {
    Crank_Type* cursor = type;
    while (cursor->rename_of) {
        cursor = cursor->rename_of;
    }
    return cursor;
}

// Crank type match is a strict type matching.
// Typecasting or promotion or implicit conversion is handled else where.
bool crank_type_match(Crank_Type* a, Crank_Type* b) {

    // Since I intern Crank_Types this is always okay.
    printf("Crank_Type a (%p) (name: %s array_dim(n=%d))\n", a, a->name.c_str(), a->rename_of, a->array_dimensions.size());
    printf("Crank_Type b (%p) (name: %s array_dim(n=%d))\n", b, b->name.c_str(), b->rename_of, b->array_dimensions.size());

    a = follow_typedef_chain(a);
    b = follow_typedef_chain(b);

    if (a == b) {
        return true;
    }

    return false;
}

// globally registered types
// TODO: make this per module. I'm just doing this to make resolution easy until I figure out what I'm doing.
std::vector<Crank_Type*> global_type_table; // too many allocations, but I don't want to deal with pointer fix up right now.

Crank_Type* register_new_type(
    std::string_view name,
    int type,
    std::vector<int> array_dimensions={},
    std::vector<Crank_Declaration> call_parameters={},
    bool is_function = false,
    int pointer_depth = 0
) {
    global_type_table.push_back(new Crank_Type);
    Crank_Type* result = global_type_table.back();
    result->type = type;
    result->name = std::string(name);
    result->array_dimensions = array_dimensions;
    result->call_parameters  = call_parameters;
    result->is_function = is_function;
    result->pointer_depth = pointer_depth;
    return result;
}

// TODO: Function type checking
/*
  NOTE: How do I consider pointers vs arrays... That'll be something
  to look forward to I guess.
*/

/*
 * NOTE: if looking up a function type or array type, it will
 * also register the new type. This is to simplify the code writing
 * since they are "derivative" types which I don't think justify having
 * to produce an explicit registration.
 *
 * This means that you don't have to typedef function pointers. Any compatible
 * function pointer according to the type system should just work. Of course when
 * transpiling to C/C++... Well that's a whole different problem isn't it :)
 *
 * I need to check the call_parameters to ensure that the declarations
 * I make are legal (We should not be allowed to declare a new DECL_TYPE)
 *
 * NOTE: need to rework this for when I actually use real modules which require lookup
 */

bool type_decl_is_derivative(
    std::string_view name,
    std::vector<int> array_dimensions={},
    std::vector<Crank_Declaration> call_parameters={},
    bool is_function = false,
    int pointer_depth = 0
) {
    return (array_dimensions.size() != 0) ||
        (call_parameters.size() != 0) ||
        (is_function) ||
        (pointer_depth != 0);
}

Crank_Type* lookup_type(
    // NOTE: should make this Crank_Type_Declaration but okay
    std::string_view name,
    std::vector<int> array_dimensions={},
    std::vector<Crank_Declaration> call_parameters={},
    bool is_function = false,
    int pointer_depth = 0
) {
    printf("Checking for %.*s, (array dimens: %d), (call params: %d), (isfunc: %d), (ptrdepth: %d)\n",
           unwrap_string_view(name),
           array_dimensions.size(),
           call_parameters.size(),
           is_function,
           pointer_depth
    );

    // check for the base type
    Crank_Type* result = nullptr;
    if (type_decl_is_derivative(name, array_dimensions, call_parameters, is_function, pointer_depth) && lookup_type(name)) {
        // checking derivative types
        for (auto type : global_type_table) {
            if (type->name != name) {
                result = nullptr;
                continue;
            }

            // possible match, start checking individual parts
            result = type;

            if (is_function == type->is_function) {
                if (type->call_parameters.size() == call_parameters.size()) {
                    for (int i = 0; i < type->call_parameters.size() && result; ++i) {
                        auto& type_param_a = type->call_parameters[i];
                        auto& type_param_b = call_parameters[i];

                        if (type_param_b.object_type != type_param_a.object_type) {
                            result = nullptr;
                        }
                    }
                } else {
                    result = nullptr;
                }
            } else {
                result = nullptr;
            }

            if (type->array_dimensions.size() == array_dimensions.size()) {
                for (int i = 0; i < type->array_dimensions.size() && result; ++i) {
                    if (type->array_dimensions[i] != array_dimensions[i]) {
                        result = nullptr;
                    }
                }
            } else {
                result = nullptr;
            }

            if (type->pointer_depth != pointer_depth) {
                result = nullptr;
                continue;
            }

            if (result) return result;
        }

        if (result == nullptr) {
            // register the derivative type.
            result = register_new_type(name, lookup_type(name)->type, array_dimensions, call_parameters, is_function, pointer_depth);
        }
    }

    if (!result) {
        for (auto type : global_type_table) {
            if (type->name == name) {
                return type;
            }
        }
    }

    return result;
}

struct Crank_Module {
    /* general purpose declaration list. */
    /* should be separated into more specific types to make things easier to look at. */
    std::string module_name; // usually just the file name. Can be overriden?
    std::vector<Crank_Module*> imports;
    std::vector<Crank_Declaration> decls;

    // hack
    bool has_main = false;
};

// This is basically just the param list for lookup_type.
// so I should just refactor this out later.
struct Crank_Type_Declaration { // NOTE: for semantic analysis. Not doing type system things here!
    std::string name;
    std::vector<int> array_dimensions = {};
    std::vector<Crank_Declaration> call_parameters = {};
    int pointer_depth = 0;
    bool is_function = false;

    bool is_derivative() {
        return ((array_dimensions.size() != 0) ||
                (call_parameters.size() != 0) ||
                (pointer_depth != 0) ||
                (is_function)); 
    }
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
    Error<Crank_Declaration> parse_variable_decl(Tokenizer_State& tokenizer);

    auto name = tokenizer.read_next();
    if (name.type != TOKEN_SYMBOL) return Error<Crank_Type_Declaration>::fail("Bad type declaration.");

    std::vector<int> array_dimensions = {};

    // parse an array type
    printf("try to find array specifier\n");
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
    printf("parsed %d array dimensions\n", array_dimensions.size());

    // parse a function type
    printf("try to find function parameters\n");
    std::vector<Crank_Declaration> call_parameters;
    bool is_function = false;
    if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
        printf("This is a function param list!\n");
        tokenizer.read_next();
        is_function = true;
        while (tokenizer.peek_next().type != TOKEN_RIGHT_PARENTHESIS) {
            printf("trying to read param.\n");
            auto decl = parse_variable_decl(tokenizer);

            if (decl.good) {
                call_parameters.push_back(decl.value);
                if (tokenizer.peek_next().type == TOKEN_RIGHT_PARENTHESIS) {
                    continue;
                } else {
                    assert(tokenizer.peek_next().type == TOKEN_COMMA && "Comma separated params list!");
                    tokenizer.read_next();
                }
            }
        }
        printf("finished param list!\n");
        assert(tokenizer.read_next().type == TOKEN_RIGHT_PARENTHESIS);
    }

    // parse pointers at the end
    int pointer_depth = 0;
    printf("Trying to parse pointers\n");
    while (tokenizer.peek_next().type == TOKEN_MUL) {
        tokenizer.read_next();
        pointer_depth += 1;
    }

    if (is_function) printf("Function decl found with %d parameters\n", call_parameters.size());
    Crank_Type_Declaration result;

    result.name             = name.string;
    result.is_function      = is_function;
    result.array_dimensions = array_dimensions;
    result.call_parameters  = call_parameters;
    result.pointer_depth    = pointer_depth;

    return Error<Crank_Type_Declaration>::okay(result);
}

/* A mathematical expression or boolean expression */
// NOTE: not comma.
enum Crank_Expression_Type {
    EXPRESSION_VALUE,
    EXPRESSION_UNARY,
    EXPRESSION_BINARY,
};

// NOTE: split expression parsing into a separate
// file
enum Crank_Expression_Operator {
    OPERATOR_NOT, // unary
    OPERATOR_NEGATE,
    OPERATOR_PROPERTY_ACCESS, // NOTE: not unary but!

    // binary...
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
    OPERATOR_MOD,

    OPERATOR_ARRAY_INDEX,

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

    OPERATOR_BITAND,
    OPERATOR_BITOR,
    OPERATOR_BITXOR,
    OPERATOR_BITNOT,
};

const char* Crank_Expression_Operator_string_table[] = {
    "!",
    "-",
    "property-access",

    "+",
    "-",
    "*",
    "/",
    "%",

    "array-index",

    "=",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",

    "!=",
    "==",
    "<",
    ">",
    "<=",
    ">=",

    "&&",
    "||",

    "&",
    "|",
    "^",
    "~",

    "count"
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
        case TOKEN_DOT: return OPERATOR_PROPERTY_ACCESS;

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

        case TOKEN_BITAND: return OPERATOR_BITAND;
        case TOKEN_BITOR:  return OPERATOR_BITOR;
        case TOKEN_BITXOR: return OPERATOR_BITXOR;
        case TOKEN_BITNOT: return OPERATOR_BITNOT;

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

Error<Crank_Declaration> read_inline_declaration(Tokenizer_State& tokenizer);

// NOTE: consider read function value!
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

// dot syntax NOTE: don't have pointer dereference yet but it should come before this!
Crank_Expression* parse_property_accessor(Tokenizer_State& tokenizer);
Crank_Expression* parse_array_index(Tokenizer_State& tokenizer);
Crank_Expression* parse_value(Tokenizer_State& tokenizer);

Crank_Expression* parse_value(Tokenizer_State& tokenizer) {
    // NOTE:
    // parethentical groupings are not stored in the tree
    // which makes it impossible to issue warnings about them right
    // now.
    if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
        tokenizer.read_next();
        auto expression = parse_expression(tokenizer);
        assert(tokenizer.peek_next().type == TOKEN_RIGHT_PARENTHESIS && "Incorrectly terminated expression!");
        tokenizer.read_next();
        return expression;
    } else {
        auto value_read = read_value(tokenizer);
        if (value_read.good) {
            return value_expression(value_read.value);
        }
    }
    return nullptr;
}

Crank_Expression* parse_array_index(Tokenizer_State& tokenizer) {
    Crank_Expression* index_expression = nullptr;
    auto              base_accessor    = parse_value(tokenizer);

    if (tokenizer.peek_next().type == TOKEN_LEFT_SQUARE_BRACE) {
        if (base_accessor == nullptr) {
            printf("warning: no base accessor?\n");
        }

        while (tokenizer.peek_next().type == TOKEN_LEFT_SQUARE_BRACE) {
            tokenizer.read_next();

            auto inner_expression = parse_expression(tokenizer);
            assert(inner_expression && "An array index operation requires an expression!");

            auto closing_brace = tokenizer.read_next();
            assert(closing_brace.type == TOKEN_RIGHT_SQUARE_BRACE && "Array index should be closed with brace!");
            // TODO: Should be checked later to see if it's an integer type.

            index_expression = binary_expression(base_accessor, inner_expression, OPERATOR_ARRAY_INDEX);
            base_accessor = index_expression;
        }

        return index_expression;
    }

    return base_accessor;
}

Crank_Expression* parse_property_accessor(Tokenizer_State& tokenizer) {
    auto base_accessor = parse_array_index(tokenizer);

    if (tokenizer.peek_next().type == TOKEN_DOT) {
        // accessor!
        // this will just be recursive.
        tokenizer.read_next(); // skip dot
        return binary_expression(base_accessor, parse_property_accessor(tokenizer), OPERATOR_PROPERTY_ACCESS);
    }

    return base_accessor;
}

Crank_Expression* parse_unary(Tokenizer_State& tokenizer) {
    auto peek_next = tokenizer.peek_next();
    if (peek_next.type == TOKEN_NOT || peek_next.type == TOKEN_SUB || peek_next.type == TOKEN_BITNOT) {
        printf("Unary found!\n");
        auto next = tokenizer.read_next(); 
        int operation = token_to_operation(next);

        // to course-correct.
        if (operation == OPERATOR_SUB) {
            operation = OPERATOR_NEGATE;
        }

        assert(operation != -1 && "something went wrong here.");
        return unary_expression(parse_property_accessor(tokenizer), operation);
        // return unary_expression(parse_array_index(tokenizer), operation);
    }

    return parse_property_accessor(tokenizer);
}

Crank_Expression* parse_factor(Tokenizer_State& tokenizer) {
    Crank_Expression* result = parse_unary(tokenizer);

    while (
        tokenizer.peek_next().type == TOKEN_MUL   ||
        tokenizer.peek_next().type == TOKEN_DIV   ||
        tokenizer.peek_next().type == TOKEN_MOD   ||
        tokenizer.peek_next().type == TOKEN_AND   ||
        tokenizer.peek_next().type == TOKEN_BITAND ||
        // TODO: I don't think this respects C operator order
        tokenizer.peek_next().type == TOKEN_BITXOR
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
        tokenizer.peek_next().type == TOKEN_ADD   ||
        tokenizer.peek_next().type == TOKEN_SUB   ||
        tokenizer.peek_next().type == TOKEN_OR    ||
        tokenizer.peek_next().type == TOKEN_BITOR
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

// I need to rewrite like 90% of this later.
enum Crank_Statement_Type {
    STATEMENT_BLOCK, // { curly brace blocks }
    STATEMENT_DECLARATION, // variable decl mostly
    STATEMENT_EXPRESSION, // the normal kind of statement
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_RETURN,

    STATEMENT_SWITCH,
    STATEMENT_CONTINUE,
    STATEMENT_BREAK,

    STATEMENT_COUNT
};
const char* Crank_Statement_Type_string_table[] = {
    [STATEMENT_BLOCK]       = "block",
    [STATEMENT_DECLARATION] = "declaration",
    [STATEMENT_EXPRESSION]  = "expression",
    [STATEMENT_IF]          = "if-statement",
    [STATEMENT_WHILE]       = "while-statement",
    [STATEMENT_RETURN]      = "return-statement",

    [STATEMENT_SWITCH]      = "switch-statement",
    [STATEMENT_CONTINUE]      = "continue-statement",
    [STATEMENT_BREAK]      = "break-statement",
};

// This allows fall through.
// NOTE: 
//  this should compile into a normal switch statement when possible
//  **IF AND ONLY IF**: I determine that the entire statement has
//  **CONSTANT** clauses.
//
// Otherwise it will compile as a chain of if_else with ors to simulate
// fall through.
struct Crank_Statement_Switch {
    Crank_Expression* condition;
    // std::vector<Case> terms;
};

struct Crank_Statement_If {
    Crank_Expression* condition;
    Crank_Statement* true_branch;
    Crank_Statement* false_branch;
};

struct Crank_Statement_While {
    Crank_Expression* condition;
    Crank_Statement* action;
};

struct Crank_Statement_Return {
    Crank_Expression* result;
};

struct Crank_Statement_Expression {
    Crank_Expression* expression;
};

struct Crank_Statement_Declaration {
    Crank_Declaration* declaration;
};

struct Crank_Statement_Block {
    std::vector<Crank_Statement*> body;
};

struct Crank_Statement {
    int type;
    // NOTE:
    // I would prefer to union this
    // but the std::vector stuff makes it complicated.
    // I might just make my own simple_vector or Array<T>
    // in order to allow union types...
    //
    // Since std::vector is not trivially constructable
    Crank_Statement_If          if_statement;
    Crank_Statement_While       while_statement;
    Crank_Statement_Return      return_statement;
    Crank_Statement_Expression  expression_statement;
    Crank_Statement_Declaration declaration_statement;
    Crank_Statement_Switch      switch_statement;
    Crank_Statement_Block       block_statement; // or a compound statement
};

Crank_Statement* parse_any_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_block_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_if_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_while_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_return_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_switch_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_expression_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_declaration_statement(Tokenizer_State& tokenizer);

Crank_Statement* parse_declaration_statement(Tokenizer_State& tokenizer) {
    Error<Crank_Declaration> parse_variable_decl(Tokenizer_State& tokenizer);
    printf("parsing inline decl\n");
    auto inline_decl = parse_variable_decl(tokenizer);
    if (!inline_decl.good) {
        printf("not a decl! fail!\n");
        return nullptr;
    }

    printf("Making new declaration\n");
    // creating heap copy of decl cause I should've thought of that
    // sooner.
    Crank_Declaration* decl = new Crank_Declaration;
    *decl = inline_decl;

    Crank_Statement* declaration_statement = new Crank_Statement;
    declaration_statement->type = STATEMENT_DECLARATION;
    declaration_statement->declaration_statement.declaration = decl;

    return declaration_statement;
}

Crank_Statement* parse_expression_statement(Tokenizer_State& tokenizer) {
    auto expression = parse_expression(tokenizer);

    if (expression) {
        Crank_Statement* expression_statement = new Crank_Statement;
        expression_statement->type = STATEMENT_EXPRESSION;
        expression_statement->expression_statement.expression = expression;
        return expression_statement;
    }

    return nullptr;
}

Crank_Statement* parse_return_statement(Tokenizer_State& tokenizer) {
    auto return_symbol = tokenizer.peek_next();
    if (return_symbol.type == TOKEN_SYMBOL) {
        if (return_symbol.string == "return") { // good
            tokenizer.read_next();

            auto expression = parse_expression(tokenizer);
            
            Crank_Statement* return_statement = new Crank_Statement;
            return_statement->type = STATEMENT_RETURN;
            return_statement->return_statement.result = expression;
            return return_statement;
        }
    }

    return nullptr;
}

Crank_Statement* parse_while_statement(Tokenizer_State& tokenizer) {
    auto while_symbol = tokenizer.peek_next();
    if (while_symbol.type == TOKEN_SYMBOL) {
        if (while_symbol.string == "while") { // good
            tokenizer.read_next();

            auto condition = parse_expression(tokenizer);
            
            Crank_Statement* while_statement = new Crank_Statement;
            while_statement->type = STATEMENT_WHILE;
            while_statement->while_statement.condition = condition;
            while_statement->while_statement.action = parse_any_statement(tokenizer);
            return while_statement;
        }
    }

    return nullptr;
}

#if 0
Crank_Statement* parse_switch_statement(Tokenizer_State& tokenizer) {
    auto switch_symbol = tokenizer.peek_next();
    if (switch_symbol.type == TOKEN_SYMBOL) {
        if (switch_symbol.string == "switch") {
            tokenizer.read_next();

            auto condition = parse_expression(tokenizer);

            Crank_Statement* switch_statement = new Crank_Statement;
            switch_statement->type = STATEMENT_SWITCH;
            switch_statement->switch_statement.condition = condition;

            // NOTE: ... hmm turns out a switch statement
            // can have many implications. Let me come back to this
            // one.
            // switch body

            return switch_statement;
        }
    }
}
#endif

Crank_Statement* parse_if_statement(Tokenizer_State& tokenizer) {
    auto if_symbol = tokenizer.peek_next();
    if (if_symbol.type == TOKEN_SYMBOL) {
        if (if_symbol.string == "if") { // good
            tokenizer.read_next();

            auto condition = parse_expression(tokenizer);
            
            Crank_Statement* if_statement = new Crank_Statement;
            if_statement->type = STATEMENT_IF;
            if_statement->if_statement.condition = condition;
            if_statement->if_statement.true_branch = parse_any_statement(tokenizer);

            auto else_symbol = tokenizer.peek_next();
            if (else_symbol.type == TOKEN_SYMBOL && else_symbol.string == "else") {
                tokenizer.read_next();
                if_statement->if_statement.false_branch = parse_any_statement(tokenizer);
            }
            return if_statement;
        }
    }

    return nullptr;
}

Crank_Statement* parse_any_statement(Tokenizer_State& tokenizer) {
    /* exhaust all statement types to parse. */
    printf("trying to parse if\n");
    auto if_statement = parse_if_statement(tokenizer);
    if (if_statement) return if_statement;

    printf("trying to parse while\n");
    auto while_statement = parse_while_statement(tokenizer);
    if (while_statement) return while_statement;

    printf("trying to parse block\n");
    auto block_statement = parse_block_statement(tokenizer);
    if (block_statement) return block_statement;

    // NOTE: semicolon requiring statements
    printf("trying to parse return\n");
    auto return_statement = parse_return_statement(tokenizer);
    if (return_statement) {
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON && "Statement requiring semicolon");
        return return_statement;
    }

    printf("trying to parse declaration\n");
    {
        int current_read_cursor = tokenizer.read_cursor;
        // hack, I need to uneat or "vomit" out a token
        auto declaration_statement = parse_declaration_statement(tokenizer);
        if (declaration_statement) {
            assert(tokenizer.read_next().type == TOKEN_SEMICOLON && "Statement requiring semicolon");
            return declaration_statement;
        } else {
            tokenizer.read_cursor = current_read_cursor;
        }
    }

    printf("trying to parse expression\n");
    auto expression_statement = parse_expression_statement(tokenizer);
    if (expression_statement) {
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON && "Statement requiring semicolon");
        return expression_statement;
    }

    return nullptr;
}

Crank_Statement* parse_block_statement(Tokenizer_State& tokenizer) {
    Crank_Statement* result = nullptr;

    if (tokenizer.peek_next().type == TOKEN_LEFT_CURLY_BRACE) {
        result = new Crank_Statement;
        result->type = STATEMENT_BLOCK;

        tokenizer.read_next();

        while (tokenizer.peek_next().type != TOKEN_RIGHT_CURLY_BRACE) {
            auto statement = parse_any_statement(tokenizer);
            if (statement) result->block_statement.body.push_back(statement);
        }
        tokenizer.read_next();

    }

    return result;
}

Error<Crank_Value> read_value(Tokenizer_State& tokenizer) {
    auto first = tokenizer.peek_next();
    Crank_Value value = {};
    value.value_type = VALUE_TYPE_LITERAL;

    switch(first.type) {
        case TOKEN_CHARACTER: {
            tokenizer.read_next();
            printf("Found char literal.\n");
            value.type = lookup_type("char");
            value.int_value = (int)first.valuechar;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_STRING: {
            tokenizer.read_next();
            printf("Found string literal.\n");
            value.type = lookup_type("strlit");
            // NOTE?
            value.string_value = first.string;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_NUMBERINT: {
            tokenizer.read_next();
            printf("Found number.\n");
            value.type = lookup_type("int");
            value.int_value = first.value32;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_NUMBERFLOAT: {
            tokenizer.read_next();
            printf("Found float number.\n");
            value.type = lookup_type("float");
            value.float_value = first.value32f;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_SYMBOL: {
            tokenizer.read_next();
            /*
              NOTE:
              Leads to one of the three possibilities

              symbol literal
              object literal
              function literal (which requires registering else where)
            */

            printf("Found symbol.\n");
            auto next = tokenizer.peek_next();
            value.value_type = VALUE_TYPE_SYMBOL;
            if (next.type == TOKEN_LEFT_CURLY_BRACE) {
                printf("Presuming this to be an object literal\n");
                // object literal
                /*
                 * NOTE: just keep it like this to be simple, so it's kinda like Go.
                 * Don't need a fancy parser. Need a working language.
                 *
                 * This isn't like C++ with initializers. Only for objects.
                 */
                tokenizer.read_next();
                value.value_type = VALUE_TYPE_LITERAL;
                value.type = lookup_type(first.string);
                assert(value.type && "This struct type should exist!");
                // NOTE: I should check if the type is indeed a struct
                // TODO causes issues with non-parenthesized things
                // need tochange syntax to avoid conflict
                Crank_Object_Literal* literal_object = new Crank_Object_Literal;
                value.literal_value = literal_object;
                {
                    /* here we go! */
                    // keep reading declarations.

                    auto first = tokenizer.peek_next();
                    if (first.type == TOKEN_SYMBOL) {
                        printf("Named literal\n");
                        literal_object->type = OBJECT_LITERAL_DECL_NAMED;

                        while (tokenizer.peek_next().type != TOKEN_RIGHT_CURLY_BRACE) {
                            printf("Reading a field\n");
                            auto field_symbol_token = tokenizer.read_next();
                            assert(field_symbol_token.type == TOKEN_SYMBOL && "This should be the name of the field!");
                            auto colon_separator_token = tokenizer.read_next();
                            assert(colon_separator_token.type == TOKEN_COLON && "This should be a colon!");

                            auto field_value = parse_expression(tokenizer);
                            assert(field_value && "Error while reading record declaration value!");
                            Crank_Object_Named_Value new_value = {std::string(field_symbol_token.string), field_value};
                            literal_object->named_values.push_back(new_value);

                            {
                                auto comma = tokenizer.peek_next();
                                // Allows optional trailing comma
                                if (comma.type == TOKEN_RIGHT_CURLY_BRACE) {
                                    continue;
                                } else {
                                    assert(comma.type == TOKEN_COMMA && "This token must be a comma.");
                                    tokenizer.read_next();
                                }
                            }
                        }
                    } else {
                        printf("Ordered literal\n");
                        literal_object->type = OBJECT_LITERAL_DECL_ORDERED;

                        while (tokenizer.peek_next().type != TOKEN_RIGHT_CURLY_BRACE) {
                            printf("Reading a field\n");
                            auto field_value = parse_expression(tokenizer);
                            assert(field_value && "Error while reading record declaration value!");
                            literal_object->expressions.push_back(field_value);

                            {
                                auto comma = tokenizer.peek_next();
                                // Allows optional trailing comma
                                if (comma.type == TOKEN_RIGHT_CURLY_BRACE) {
                                    continue;
                                } else {
                                    assert(comma.type == TOKEN_COMMA && "This token must be a comma.");
                                    tokenizer.read_next();
                                }
                            }
                        }
                    }
                }
                assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
                return Error<Crank_Value>::okay(value);
            } else {
                // symbol
                value.symbol_name = first.string;

                /* handle reserved keywords */
                if (value.symbol_name == "null") {
                    assert(0 && "reserved keyword 'null'! Aborting!");
                } else if (value.symbol_name == "true") {
                    value.value_type = VALUE_TYPE_LITERAL;
                    value.type = lookup_type("bool");
                    value.int_value = 1;
                } else if (value.symbol_name == "false") {
                    value.value_type = VALUE_TYPE_LITERAL;
                    value.type = lookup_type("bool");
                    value.int_value = 0;
                } else {
                    if (next.type == TOKEN_LEFT_PARENTHESIS) {
                        printf("Possible function call\n");
                        tokenizer.read_next(); 

                        value.is_function_call = true;
                        while (tokenizer.peek_next().type != TOKEN_RIGHT_PARENTHESIS) {
                            auto new_value = parse_expression(tokenizer);
                            assert(new_value && "Bad function param passing");
                            value.call_parameters.push_back(new_value);

                            if (tokenizer.peek_next().type == TOKEN_COMMA) {
                                tokenizer.read_next();
                                printf("HI I ATE A COMMA TODAY!\n");
                            } else if (tokenizer.peek_next().type == TOKEN_RIGHT_PARENTHESIS) {
                                break;
                            } else {
                                assert(0 && "Bad call params list! Needs comma separated parameters");
                            }
                        }
                        tokenizer.read_next();
                    }
                }
                return Error<Crank_Value>::okay(value);
            }
        } break;
            // array literal!
            // NOTE: not type checked here.
            // we'll type check values after they're read.
        case TOKEN_LEFT_SQUARE_BRACE: {
            tokenizer.read_next();
            // literal type.
            while (tokenizer.peek_next().type != TOKEN_RIGHT_SQUARE_BRACE) {
                auto new_value = parse_expression(tokenizer);
                assert(new_value && "Bad array literal parsing?");
                value.array_elements.push_back(new_value);

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
        case TOKEN_COMMENT: { tokenizer.read_next(); } break;
        default: {
            printf("Encountered invalid token type for value: %.*s\n", unwrap_string_view(Token_Type_string_table[first.type]));
        } break;
    }

    return Error<Crank_Value>::fail("bad value");
}

// TODO: Does not type check yet.
bool do_array_typecheck(Crank_Type* type,
                        std::vector<int> array_dimensions,
                        std::vector<Crank_Expression*> array_elements) {
    auto expected_element_count = array_dimensions[0];

    // -1 means flexible. So it will always pass that check.
#if 0
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
#else
    // need to do this later.
    return true;
#endif

}

// Any form that resembles a variable declaration.
/*
 *
 * NAME : (decorators? like extern/const/modifiers) (typedecl)
 */
Error<Crank_Declaration> read_inline_declaration(Tokenizer_State& tokenizer) {
    Crank_Declaration result = {};

    result.decl_type = DECL_OBJECT;
    result.has_value = false;

    if (tokenizer.peek_next().type != TOKEN_SYMBOL) {
        return Error<Crank_Declaration>::fail("Not a declaration");
    } 

    auto name = tokenizer.peek_next();
    if (name.type != TOKEN_SYMBOL) return Error<Crank_Declaration>::fail("not a symbol, cannot find name");
    tokenizer.read_next();
    auto colon = tokenizer.peek_next();
    if (colon.type != TOKEN_COLON) return Error<Crank_Declaration>::fail("not a colon, cannot be a decl");
    tokenizer.read_next();

    auto extern_token = tokenizer.peek_next();
    printf("Trying to parse for extern(%.*s)\n", unwrap_string_view(extern_token.string));
    // if I were to output native code, I'd assume standard C linkage for now.
    if (extern_token.type == TOKEN_SYMBOL && extern_token.string == "extern") {
        tokenizer.read_next();
        printf("Object is an extern declaration!\n");
        /*
         * This is technically more important for when we need to check if all functions are defined.
         */
        result.is_externally_defined = true;

        if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
            tokenizer.read_next();
            auto extern_linkage_name = tokenizer.read_next();
            assert(extern_linkage_name.type == TOKEN_STRING);
            result.extern_definition.linkage_name = extern_linkage_name.stringvalue;
            printf("This object is linked as \"%s\"", result.extern_definition.linkage_name.c_str());
            assert(tokenizer.read_next().type == TOKEN_RIGHT_PARENTHESIS && "Extern name param needs to be closed with a parenthesis!");
        }
    }

    auto type_entry = read_type_declaration(tokenizer);
    assert(type_entry.good && "Bad type entry?");
    printf("Checking decl type... (%s)\n", type_entry.value.name.c_str());
    /* NOTE: replace this stuff. Should be a bit faster... */
    auto type = lookup_type(
        type_entry.value.name,
        type_entry.value.array_dimensions,
        type_entry.value.call_parameters,
        type_entry.value.is_function,
        type_entry.value.pointer_depth
    );
    assert(type && "Type not found! Cannot resolve!");

    result.array_dimensions = type_entry.value.array_dimensions;
    result.object_type = type; // NOTE? need to check something
    result.name = name.string;

    if (type_entry.value.is_function) {
        assert(type->is_function);
        printf("This decl is a function!\n");
        auto function_statement = parse_any_statement(tokenizer);
        if (function_statement) {
            printf("function declaration!\n");
            result.has_value = true;
            // result.value.body = function_statement;
            result.expression = new Crank_Expression;
            result.expression->type = EXPRESSION_VALUE;
            result.expression->value.body = function_statement;
        }
    } else {
        printf("This decl is a variable!\n");
        if (tokenizer.peek_next().type == TOKEN_EQUAL) {
            tokenizer.read_next();
            auto value = parse_expression(tokenizer);
            assert(value);
            result.has_value = true;
            result.expression = value;

            // assert that the evaluated type should match the type we
            // found.

            // NOTE: arrays have to be typed here. We'll enforce a type check on all
            // elements which is kind of slow since I should've tested it while parsing?
            // however the code is not organized to allow this for now.

#if 0 // disabled for now TODO: array typecheck against known size
            if (result.value.array_elements.size() > 0) {
                printf("Array type checking\n");
                result.value.type = type;

                std::vector<int> array_dimensions = result.value.type->array_dimensions;
                assert(do_array_typecheck(type, array_dimensions, value.value.array_elements) && "check the message.");
            }
#endif

#if 0 // will not work currently since expressions are nested quite thoroughly
            assert(value.value.type && "Value does not have a type for some reason?");
#endif

            // This is not being done yet.

            // NOTE: for arrays this is loser. As long as long as it can
            // be cast to that thing.
        }
    }

    return Error<Crank_Declaration>::okay(result);
}

bool read_record_definition(Crank_Type* type, Tokenizer_State& tokenizer) {
    assert(type->type == TYPE_RECORD && "wtf");
    printf("Trying to read decl\n");
    while (tokenizer.peek_next().type == TOKEN_SYMBOL) { // I don't think I need to check this anymore b/c read_inline_declaration returns error but okay
        auto new_member_decl = read_inline_declaration(tokenizer).value;
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON);
        // assert(new_member_decl.good && "Bad decl");
        // TODO: check for duplicates!
        if (new_member_decl.has_value) {
            printf("Sorry! No default value yet!\n");
        }
        // or delegates I guess
        // NOTE: function pointer members not working
        Crank_Declaration member;
        member.name             = new_member_decl.name;
        member.array_dimensions = new_member_decl.array_dimensions;
        member.object_type      = new_member_decl.object_type;

        type->members.push_back(member);
    }
    printf("Finished reading decl! (%d members?)\n", type->members.size());
    assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
    return true;
}

Error<Crank_Declaration> parse_typedef(Tokenizer_State& tokenizer) {
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
        typedecl.decl_type = DECL_TYPE;
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
        printf("read typedef rename\n");
        return Error<Crank_Declaration>::okay(typedecl);
    } else if (determiner.type == TOKEN_LEFT_CURLY_BRACE) {
        // record type.
        tokenizer.read_next();

        Crank_Declaration typedecl;
        typedecl.decl_type = DECL_TYPE;
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

Error<Crank_Declaration> parse_variable_decl(Tokenizer_State& tokenizer) {
    auto inline_decl = read_inline_declaration(tokenizer);
    if (!inline_decl.good) {
        return Error<Crank_Declaration>::fail("No declaration found?");
    }

    return Error<Crank_Declaration>::okay(inline_decl.value);
}

/**
 * NOTE: 4/29/
 *
 * With more clear ideas on how to make this still allow for correct compilation behavior
 * whilst still being simple I'll do this.
 *
 *
 * Parse Types first
 * the type system is used to check if a declaration is legal (does the type of something exist?), and therefore
 * a separate pass to parse all the types is needed.
 *
 * lookup_type, will also have to get access to the module tree. In order to parse things.
 * It'll be complicated because types are not classified as expression objects so they'll require slightly special parsing
 * in order to qualify their names.
 *
 * IE:
 * name : ModuleName.Type = ModuleName.GLOBAL_VARIABLE + 34;
 *
 * However Crank is still a toy language, not meant for production and for now I can allow modules to be single level
 * which simplifies a lot of cases.
 *
 * Module name can be specified through
 * modname NewModuleName;
 *
 * The new module name cannot include dots.
 *
 * This should make it useful enough to be practical, but then falls under Java's weakness of one file per "module"
 **/
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
                    auto new_typedef = parse_typedef(tokenizer);
                    if (!new_typedef.good) {
                        printf("%s\n", new_typedef.message);
                    } else 
                        module.decls.push_back(new_typedef);
                } else {
                    // TODO: Check for duplicates (IN THE SAME SCOPE!)
                    auto new_decl = parse_variable_decl(tokenizer);
                    if (!new_decl.good) {
                        printf("%s\n", new_decl.message);
                    } else  {
                        printf("new decl added\n");
                        if (new_decl.value.object_type->is_function && new_decl.value.name == "main") {
                            printf("Found main function!\n");
                            module.has_main = true;
                        }
                        module.decls.push_back(new_decl);
                    }
                    // this causes an extra semi-colon
                    assert(tokenizer.read_next().type == TOKEN_SEMICOLON);
                }
            } break;
            case TOKEN_NONE: {
                tokenizer.read_next();
                // safe, is EOF
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
    register_new_type("int",   TYPE_INTEGER32);
    register_new_type("uint",  TYPE_UNSIGNEDINTEGER32);

    register_new_type("s64",   TYPE_INTEGER64);
    register_new_type("s32",   TYPE_INTEGER32);
    register_new_type("s16",   TYPE_INTEGER16);
    register_new_type("s8",    TYPE_INTEGER8);
    register_new_type("u64",   TYPE_UNSIGNEDINTEGER64);
    register_new_type("u32",   TYPE_UNSIGNEDINTEGER32);
    register_new_type("u16",   TYPE_UNSIGNEDINTEGER16);
    register_new_type("u8",    TYPE_UNSIGNEDINTEGER8);

    register_new_type("float", TYPE_FLOAT);
    register_new_type("double", TYPE_DOUBLE);

    register_new_type("f32", TYPE_FLOAT);
    register_new_type("f64", TYPE_DOUBLE);

    register_new_type("bool",  TYPE_BOOLEAN);
    register_new_type("char",  TYPE_CHAR);
    register_new_type("strlit",TYPE_STRINGLITERAL);
    register_new_type("void",  TYPE_VOID);
}

// NOTE: codegen explicitly refers output targets that are not
// the interpreter.
#include "codegen.cc"
#include "cplusplus_codegen.cc"

#include "os_process_win32.c"

#include "debug_print.cc"
#include "crank_parsing_tests.cc"

int main(int argc, char** argv){
    register_default_types();

    // TODO: actual command line arguments.
#if 1
    // NOTE: should run as a test argument only
    run_all_tests();
#else
    std::vector<std::string> module_names;
    Crank_Codegen* generator = new CPlusPlusCodeGenerator();
    for (int i = 1; i < argc; ++i) {
        std::string filename = argv[i];
        File_Buffer buffer = File_Buffer(filename.c_str());

        // TODO: I don't want to do this right now...
        std::string module_name_hack;
        for (auto& c : filename) {
            if (c == '.') break;
            module_name_hack += c;
        }
        auto e = load_module_from_source(module_name_hack, buffer.data);

        if (e.good) {
            printf("okay! outputting module!\n");
            generator->output_module(e);
        } else {
            printf("not good! module bad!\n");
            printf("%s\n", e.message);
            break;
        }

        module_names.push_back(generator->get_module_compiled_name(e));
    }

    if (module_names.size()) {
        // generated... now compile all the modules
        std::string compile_string = "g++ -o test ";
        for (auto s : module_names) {
            compile_string += s;
        }

        printf("calling c compiler\n");
        os_process_shell_start_and_run_synchronously((char*)compile_string.c_str());
        printf("enjoy.\n");
    }

    return 0;
#endif
}
