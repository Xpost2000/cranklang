/*
  A small programming language transpiler. It's not really efficient
  it's just to figure out how to write a language by any means
  necessary. So there's an allocation festival thanks to all the
  std::string I use.

  NOTE: I'm not deleting anything here because
  the OS will reclaim all memory, and it's only running until
  the compiler finishes. It's not real time so that's fine.

  OPTIONAL/TODO: I would like better error reporting logic, but this isn't really about
  that (IE: the intent isn't to make a sane language to use, it's just "compiler" practice)

  TODO: 
  - Reserve key words
  - Replace most instances of std::string
  There's too much copying that I'm not super comfortable with,
  but that's part of the clean up code.

  NOTE: A lot of thing in the "compiler" are not
  typechecked as of now! I am checking these functional changes
  in but invalid types are still allowed
  - Function decls will definitely not be typechecked yet
  - Expressions are not properly type checked!

  NOTE: have to refactor like all of this later...
  because it's not object oriented in the traditional sense.

  Code is kind of messy because it's a "hack" project.

  I'll try to clean it up and at the very least have some decent error
  propagation / reporting.
  
  NOTE: in transpiler mode, I don't have to evaluate the expressions... I just need
  to make sure they evaluate to the right type.

  Most of this code is "backend"/"compiler" logic. 

  A minimally useful language?

  TODO: Proper error handling takes a lot of code, and I don't have that
  much time so I've kinda stopped doing it at some point just to get sutff done
  faster.

  NOTE: when I add multiple modules, all Crank_Declarations are going to specify which module they
  come from.

  NOTE: Crank is not going to support methods. It's not part of the MO, although it's not additionally
  complicated...
  NOTE: Crank is not supporting multiple declarations either. It probably could with slight changes to the parser
  but it's not something I usually do or like...

  NOTE: I use a lot of megastructs since it's simpler, but inheritance is possible to reduce
  size of structs. But this probably doesn't matter too much in a lot of cases I need. Besides it's
  quicker for me to learn how to write a compiler this way anyways.

  TODO: add a formal "error" reporter and avoid asserting so much
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

    /* Not implemented yet! */
    TYPE_UNION,
    TYPE_ENUMERATION,

    // built in tagged union or algebraic data type
    TYPE_VARIANT,
    /* Not impleemented yet! */

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

struct Crank_Enum_KeyValue {
    std::string name;
    int64_t value;
};

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
    bool is_variadic = false;
    std::vector<Crank_Declaration> call_parameters;

    /*
     * This is for the "tagged" types like
     * union/variant/struct/enum
     *
     * For enum, I will check to make sure that these are some form of integer type.
     */
    Crank_Type* enum_internal_type = nullptr;
    std::vector<Crank_Enum_KeyValue> enum_members;
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

bool is_type_numeric(Crank_Type* type) {
    type = follow_typedef_chain(type);

    switch (type->type) {
        case TYPE_NUMERIC:
        case TYPE_BOOLEAN:
        case TYPE_INTEGER8: case TYPE_INTEGER16:
        case TYPE_INTEGER32: case TYPE_INTEGER64:
        case TYPE_UNSIGNEDINTEGER8: case TYPE_UNSIGNEDINTEGER16:
        case TYPE_UNSIGNEDINTEGER32: case TYPE_UNSIGNEDINTEGER64:
        case TYPE_FLOAT: case TYPE_DOUBLE:
        case TYPE_ENUMERATION:
            return true;
        default: break;
    }

    return false;
}
bool is_type_integer(Crank_Type* type) {
    type = follow_typedef_chain(type);

    switch (type->type) {
        case TYPE_BOOLEAN:
        case TYPE_INTEGER8: case TYPE_INTEGER16:
        case TYPE_INTEGER32: case TYPE_INTEGER64:
        case TYPE_UNSIGNEDINTEGER8: case TYPE_UNSIGNEDINTEGER16:
        case TYPE_UNSIGNEDINTEGER32: case TYPE_UNSIGNEDINTEGER64:
        case TYPE_ENUMERATION:
            return true;
        default: break;
    }

    return false;
}
bool is_type_unsigned_integer(Crank_Type* type) {
    type = follow_typedef_chain(type);

    switch (type->type) {
        case TYPE_BOOLEAN:
        case TYPE_UNSIGNEDINTEGER8: case TYPE_UNSIGNEDINTEGER16:
        case TYPE_UNSIGNEDINTEGER32: case TYPE_UNSIGNEDINTEGER64:
            return true;
        default: break;
    }

    return false;
}

// will return the "greater" of two types
// implicit conversions.
Crank_Type* numeric_type_promotion(Crank_Type* a, Crank_Type* b, Crank_Type* maximum_promoted_type) {
    unimplemented("numeric_type_promotion");
}

// Crank type match is a strict type matching.
// Typecasting or promotion or implicit conversion is handled else where.
bool crank_type_match(Crank_Type* a, Crank_Type* b) {

    // Since I intern Crank_Types this is always okay.
    _debugprintf("Crank_Type a (%p) (name: %s array_dim(n=%d))\n", a, a->name.c_str(), a->rename_of, a->array_dimensions.size());
    _debugprintf("Crank_Type b (%p) (name: %s array_dim(n=%d))\n", b, b->name.c_str(), b->rename_of, b->array_dimensions.size());

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
    bool is_variadic = false,
    int pointer_depth = 0
) {
    global_type_table.push_back(new Crank_Type);
    Crank_Type* result = global_type_table.back();
    result->type = type;
    result->name = std::string(name);
    result->array_dimensions = array_dimensions;
    result->call_parameters  = call_parameters;
    result->is_function = is_function;
    result->is_variadic = is_variadic;
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
    bool is_variadic = false,
    int pointer_depth = 0
) {
    _debugprintf("Checking for %.*s, (array dimens: %d), (call params: %d), (isfunc: %d), (ptrdepth: %d)\n",
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
                    if (type->is_variadic != is_variadic) {
                        result = nullptr;
                    }

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
            result = register_new_type(name, lookup_type(name)->type, array_dimensions, call_parameters, is_function, is_variadic, pointer_depth);
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

Crank_Declaration* crank_module_find_function_decl(Crank_Module& module, char* function_name) {
    for (auto& decl : module.decls) {
        if (decl.decl_type != DECL_OBJECT) {
            continue;
        }

        if (decl.name != function_name) {
            continue;
        }

        if (!decl.object_type->is_function) {
            continue;
        }

        return &decl;
    }

    return nullptr;
}

// Looking up some base types
/*
 * Unions and records are similar in codegen other than memory storage.
 * So you can search for them using the same procedure, since other than practical
 * usage they work identically.
 */
Crank_Type* crank_type_system_find_record_or_union_decl(char* type_name) {
    auto ptr = lookup_type(type_name);
    if (ptr->type == TYPE_RECORD || ptr->type == TYPE_UNION)
        return ptr;

    return nullptr;
} 

Crank_Type* crank_type_system_find_enum_decl(char* type_name) {
    auto ptr = lookup_type(type_name);
    if (ptr->type == TYPE_ENUMERATION)
        return ptr;

    return nullptr;
}

// This is basically just the param list for lookup_type.
// so I should just refactor this out later.
struct Crank_Type_Declaration { // NOTE: for semantic analysis. Not doing type system things here!
    std::string name;
    std::vector<int> array_dimensions = {};
    std::vector<Crank_Declaration> call_parameters = {};
    int pointer_depth = 0;
    bool is_function = false;
    bool is_variadic = false;
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
    _debugprintf("try to find array specifier\n");
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
    _debugprintf("parsed %d array dimensions\n", array_dimensions.size());

    // parse pointers at the end
    int pointer_depth = 0;
    _debugprintf("Trying to parse pointers\n");
    while (tokenizer.peek_next().type == TOKEN_MUL) {
        tokenizer.read_next();
        pointer_depth += 1;
    }

    // parse a function type
    _debugprintf("try to find function parameters\n");
    std::vector<Crank_Declaration> call_parameters;
    bool is_function = false;
    bool is_variadic = false;
    if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
        _debugprintf("This is a function param list!\n");
        tokenizer.read_next();
        is_function = true;
        while (tokenizer.peek_next().type != TOKEN_RIGHT_PARENTHESIS) {
            _debugprintf("trying to read param.\n");
            if (tokenizer.peek_next().type == TOKEN_DOT) {
                // possible variadic?
                assert(tokenizer.read_next().type == TOKEN_DOT && "This should've been a variadic argument?");
                assert(tokenizer.read_next().type == TOKEN_DOT && "This should've been a variadic argument?");
                assert(tokenizer.read_next().type == TOKEN_DOT && "This should've been a variadic argument?");
                // variadic arguments will follow the same ordering as C, where they can be any type I suppose
                assert(tokenizer.peek_next().type == TOKEN_RIGHT_PARENTHESIS && "A variadic argument is the end of a parameter list!");
                is_variadic = true;
            } else {
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
        }
        _debugprintf("finished param list!\n");
        assert(tokenizer.read_next().type == TOKEN_RIGHT_PARENTHESIS);
    }

    if (is_function) _debugprintf("Function decl found with %d parameters\n", call_parameters.size());
    Crank_Type_Declaration result;

    result.name             = name.string;
    result.is_function      = is_function;
    result.is_variadic      = is_variadic;
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
    OPERATOR_ADDRESSOF,
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
    "&",
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

// Expression check helpers
bool is_constant_expression(Crank_Expression* expression);
bool is_expression_numeric(Crank_Expression* expression);

/*
 * Constant values are literals for now for simplicity. But they should extend for any "const"
 * thing that I can ensure is constant.
 *
 * Maybe functions that are also marked "const", however they must not have
 * external linkage because Crank doesn't have an FFI and cannot execute arbitrary functions
 * at compile/runtime yet (or ever, since it's a pretty massive undertaking without libffi...)
 *
 * I can "polyfill" certain functions maybe (like sin, cos, pow), basically any math
 */
bool is_value_expression_constant(Crank_Expression* expression) {
    auto& value = expression->value;
    if (value.value_type == VALUE_TYPE_LITERAL) {
        // NOTE: function should take priority
        // and this should take priority?
        // need to really investigate how this will behave when testing.
#if 0
        if (value.array_elements.size()) {
            // if it's an array we'll have to just check if each array element is constant.
            for (auto& element : value.array_elements) {
                if (!is_constant_expression(element)) {
                    return false;
                }
            }

            return true;
        }
#endif

        // NOTE: if there is a symbol name. this might not work.

        // check for function?
        auto object_type = follow_typedef_chain(value.type);

        {
            if (is_type_numeric(object_type)) {
                return true;
            } else {
                // record or union
                // or otherwise. Something kind of nasty.
                if (object_type->type == TYPE_RECORD ||
                    object_type->type == TYPE_UNION) {
                    auto object_literal = value.literal_value;
                    if (object_literal->type == OBJECT_LITERAL_DECL_ORDERED) {
                        for (auto& member : object_literal->expressions) {
                            if (!is_constant_expression(member)) {
                                return false;
                            }
                        }

                        return true;
                    } else {
                        for (auto& member : object_literal->named_values) {
                            if (!is_constant_expression(member.expression)) {
                                return false;
                            }
                        }

                        return true;
                    }
                    unimplemented("Do not know how to check for constant record/union");
                }
            }
        }
    }
}

bool is_unary_expression_constant(Crank_Expression* expression) {
    auto& unary = expression->unary;

    if (expression->operation == OPERATOR_ARRAY_INDEX ||
        expression->operation == OPERATOR_ADDRESSOF) {
        // by definition these are not "constant" expressions
        // (well okay, array-index *can* be numeric, but for now I'll say they aren't to be safe)
        return false;
    }

    return is_constant_expression(unary.value);
}

bool is_binary_expression_constant(Crank_Expression* expression) {
    auto& binary = expression->binary;
    if (expression->operation == OPERATOR_EQUAL ||
        expression->operation == OPERATOR_ADDEQUAL ||
        expression->operation == OPERATOR_SUBEQUAL ||
        expression->operation == OPERATOR_MULEQUAL ||
        expression->operation == OPERATOR_DIVEQUAL ||
        expression->operation == OPERATOR_MODEQUAL) {
        return false;
    }

    // This has to be special-cased for enums, because enums **are**
    // constant values.
    // NOTE: can also be used for if records have associated static values
    // but that's not something I'm into, so I'll avoid it.
    if (expression->operation == OPERATOR_PROPERTY_ACCESS) {
        unimplemented("TODO: check if this is an enum!");
        return false;
    }

    return is_constant_expression(binary.first) && is_constant_expression(binary.second);
}

bool is_constant_expression(Crank_Expression* expression) {
    if (!expression)
        return true;

    switch (expression->type) {
        case EXPRESSION_VALUE: return is_value_expression_constant(expression);
        case EXPRESSION_UNARY: return is_unary_expression_constant(expression);
        case EXPRESSION_BINARY: return is_binary_expression_constant(expression);
    }

    return false;
}

bool is_value_expression_numeric(Crank_Expression* expression) {
    auto& value = expression->value;
    auto object_type = follow_typedef_chain(value.type);

    // thankfully this is pretty simple...
    // NOTE: if it's a function call... I need to check what type
    // function calls are.

    if (value.array_elements.size() > 0) return false;

    return is_type_numeric(object_type);
}

bool is_unary_expression_numeric(Crank_Expression* expression) {
    auto& unary = expression->unary;
    return is_expression_numeric(unary.value);
}

bool is_binary_expression_numeric(Crank_Expression* expression) {
    auto& binary = expression->binary;

    // This has to be special-cased for enums, because enums **are**
    // also numeric values.
    if (expression->operation == OPERATOR_PROPERTY_ACCESS) {
        auto& first = expression->binary.first;
        if (first->type == EXPRESSION_VALUE &&
            first->value.value_type == VALUE_TYPE_SYMBOL) {
            auto& symbol_name = first->value.symbol_name;
            auto  enum_type    = crank_type_system_find_enum_decl((char*)symbol_name.c_str());

            if (enum_type) {
                auto& second = expression->binary.second;

                assert(
                    second->type == EXPRESSION_VALUE &&
                    second->value.value_type == VALUE_TYPE_SYMBOL &&
                    "enum value access error! Second param of property access should be the value!"
                );

                return true;
            }
        }

        return false;
    }

    return is_expression_numeric(binary.first) && is_expression_numeric(binary.second);
}

bool is_expression_numeric(Crank_Expression* expression) {
    if (!expression)
        return true;
    
    switch (expression->type) {
        case EXPRESSION_VALUE: return is_value_expression_numeric(expression);
        case EXPRESSION_UNARY: return is_unary_expression_numeric(expression);
        case EXPRESSION_BINARY: return is_binary_expression_numeric(expression);
    }
    return false;
}

// NOTE this will determine the type from the first
// evaluated sub-expression
// Also handles type promotion with a strict flag
// NOTE: This allows type inference I suppose.
Crank_Type* get_expression_type(Crank_Expression* expression, bool strict=false) {
    unimplemented("get_expression_type");
}

// NOTE: will only work with numeric types for now
// folding arrays should be possible in the future.
// maybe even strings? Concatenation is pretty well defined
Crank_Expression* fold_constant_numeric_expression(Crank_Expression* expression);

Crank_Expression* fold_constant_numeric_unary_expression(Crank_Expression* expression) {
    auto& unary = expression->unary;
    // unimplemented("fold_constant_numeric_unary_expression not implemented yet!");
    Crank_Expression* result = new Crank_Expression;
    result->type = EXPRESSION_VALUE;
    result->value = expression->value;
    if (expression->operation == OPERATOR_NEGATE) {
        if (is_type_integer(result->value.type)) {
            assert(is_type_unsigned_integer(result->value.type) && "NOTE: negating an unsigned number is not defined?");
            result->value.int_value *= -1;
        }
    } else {
        assert(false && "undefined operation behavior for now!");
    }
    return result;
}
Crank_Expression* fold_constant_numeric_binary_expression(Crank_Expression* expression) {
    auto& binary = expression->binary;
    unimplemented("fold_constant_numeric_binary_expression not implemented yet! I require numeric_type_promotion!");
}
Crank_Expression* fold_constant_numeric_expression(Crank_Expression* expression) {
    if (!is_constant_expression(expression)) return nullptr;
    if (!is_expression_numeric(expression)) return nullptr;

    switch (expression->type) {
        case EXPRESSION_VALUE:  return expression;
        case EXPRESSION_UNARY:  return fold_constant_numeric_unary_expression(expression);
        case EXPRESSION_BINARY: return fold_constant_numeric_binary_expression(expression);
    }

    return nullptr;
}

// End of Expression check helpers

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
    if (peek_next.type == TOKEN_NOT || peek_next.type == TOKEN_SUB || peek_next.type == TOKEN_BITNOT || peek_next.type == TOKEN_BITAND) {
        _debugprintf("Unary found!\n");
        auto next = tokenizer.read_next(); 
        int operation = token_to_operation(next);

        // to course-correct.
        if (operation == OPERATOR_SUB) {
            operation = OPERATOR_NEGATE;
        } else if (operation == OPERATOR_BITAND) {
            operation = OPERATOR_ADDRESSOF;
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
    STATEMENT_FOR,
    STATEMENT_WHILE,
    STATEMENT_RETURN,

    STATEMENT_CONTINUE,
    STATEMENT_BREAK,

    STATEMENT_COUNT
};
const char* Crank_Statement_Type_string_table[] = {
    [STATEMENT_BLOCK]       = "block",
    [STATEMENT_DECLARATION] = "declaration",
    [STATEMENT_EXPRESSION]  = "expression",
    [STATEMENT_IF]          = "if-statement",
    [STATEMENT_FOR]          = "for-statement",
    [STATEMENT_WHILE]       = "while-statement",
    [STATEMENT_RETURN]      = "return-statement",

    // Continue and break should allow labels as well.
    // when I add them of course.
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

/*
  NOTE: due to the behavior of this for loop,
  it is impossible to compile into a C for loop, and this
  will **always** be a while loop.

  (the initialization statements can make any variables of any type.
  Which is not legal in a normal for loop)
 */
struct Crank_Statement_For {
    std::vector<Crank_Statement*> initialization_statements;
    Crank_Expression* condition = nullptr;
    std::vector<Crank_Statement*> postloop_statements;
    Crank_Statement* body = nullptr;
};

struct Crank_Statement_While {
    Crank_Expression* condition = nullptr;
    Crank_Statement* action = nullptr;
};

struct Crank_Statement_Return {
    Crank_Expression* result = nullptr;
};

struct Crank_Statement_Expression {
    Crank_Expression* expression = nullptr;
};

struct Crank_Statement_Declaration {
    Crank_Declaration* declaration = nullptr;
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
    Crank_Statement_For         for_statement;
    Crank_Statement_Return      return_statement;
    Crank_Statement_Expression  expression_statement;
    Crank_Statement_Declaration declaration_statement;
    Crank_Statement_Block       block_statement; // or a compound statement
};

Crank_Statement* parse_any_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_block_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_if_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_for_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_while_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_return_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_expression_statement(Tokenizer_State& tokenizer);
Crank_Statement* parse_declaration_statement(Tokenizer_State& tokenizer);

Crank_Statement* parse_declaration_statement(Tokenizer_State& tokenizer) {
    Error<Crank_Declaration> parse_variable_decl(Tokenizer_State& tokenizer);
    _debugprintf("parsing inline decl\n");
    auto inline_decl = parse_variable_decl(tokenizer);
    if (!inline_decl.good) {
        _debugprintf("not a decl! fail!\n");
        return nullptr;
    }

    _debugprintf("Making new declaration\n");
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
    if (return_symbol.type == TOKEN_SYMBOL && return_symbol.string == "return") {
        tokenizer.read_next();

        auto expression = parse_expression(tokenizer);
            
        Crank_Statement* return_statement = new Crank_Statement;
        return_statement->type = STATEMENT_RETURN;
        return_statement->return_statement.result = expression;
        return return_statement;
    }

    return nullptr;
}

Crank_Statement* parse_for_statement(Tokenizer_State& tokenizer) {
    auto for_symbol = tokenizer.peek_next();

    if (for_symbol.type == TOKEN_SYMBOL && for_symbol.string == "for") {
        tokenizer.read_next();
        // try to only read declarations;
        Crank_Statement* for_statement = new Crank_Statement;
        for_statement->type = STATEMENT_FOR;

        auto& for_statement_data = for_statement->for_statement;
        bool needs_closing_parenthesis = false;

        if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
            needs_closing_parenthesis = true;
            tokenizer.read_next();
        }

        {
            bool terminate = false;
            while (!terminate) {
                _debugprintf("trying to parse declaration\n");
                int current_read_cursor = tokenizer.read_cursor;
                auto declaration_statement = parse_declaration_statement(tokenizer);
                if (declaration_statement) {
                    for_statement_data.initialization_statements.push_back(declaration_statement);
                } else {
                    tokenizer.read_cursor = current_read_cursor;
                }

                auto next = tokenizer.read_next();

                if (next.type == TOKEN_SEMICOLON) {
                    // terminate;
                    terminate = true;
                } else if (next.type == TOKEN_COMMA) {
                    continue;
                } else {
                    assert(0 && "Illegal next token read?");
                }
            }
        }
        for_statement_data.condition = parse_expression(tokenizer);
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON && "You need a semicolon after the condition!");
        {
            bool terminate = false;
            while (!terminate) {
                int current_read_cursor = tokenizer.read_cursor;
                auto any_statement = parse_expression_statement(tokenizer);

                if (!any_statement) { 
                    _debugprintf("Rewind. Failed to read expression!");
                    tokenizer.read_cursor = current_read_cursor;
                    any_statement = parse_if_statement(tokenizer);
                }

                if (any_statement) { // if is allowed because this language has no ternary operators.
                    for_statement_data.postloop_statements.push_back(any_statement);
                }

                auto next = tokenizer.peek_next();

                if (next.type == TOKEN_COMMA) {
                    tokenizer.read_next();
                    continue;
                } else {
                    terminate = true;
                }
            }
        }

        if (needs_closing_parenthesis) {
            assert(
                tokenizer.read_next().type == TOKEN_RIGHT_PARENTHESIS &&
                "You need closing parenthesis for this for loop!"
            );
        }

        for_statement_data.body = parse_block_statement(tokenizer);
        assert(for_statement_data.body && "A for loop MUST have a block body. You cannot have empty for loops!");

        _debugprintf("For loop: %d initializations, %d post actions", for_statement_data.initialization_statements.size(), for_statement_data.postloop_statements.size());
        _debugprintf("For loop: %p condition? %p body?\n", for_statement_data.condition, for_statement_data.body);

        return for_statement;
    }

    return nullptr;
}

Crank_Statement* parse_while_statement(Tokenizer_State& tokenizer) {
    auto while_symbol = tokenizer.peek_next();
    if (while_symbol.type == TOKEN_SYMBOL && while_symbol.string == "while") {
        tokenizer.read_next();

        auto condition = parse_expression(tokenizer);
            
        Crank_Statement* while_statement = new Crank_Statement;
        while_statement->type = STATEMENT_WHILE;
        while_statement->while_statement.condition = condition;
        while_statement->while_statement.action = parse_any_statement(tokenizer);
        return while_statement;
    }

    return nullptr;
}

Crank_Statement* parse_if_statement(Tokenizer_State& tokenizer) {
    auto if_symbol = tokenizer.peek_next();
    if (if_symbol.type == TOKEN_SYMBOL && if_symbol.string == "if") {
        tokenizer.read_next();

        auto condition = parse_expression(tokenizer);
            
        Crank_Statement* if_statement = new Crank_Statement;
        if_statement->type = STATEMENT_IF;
        if_statement->if_statement.condition = condition;
        // if_statement->if_statement.true_branch = parse_any_statement(tokenizer);
        if_statement->if_statement.true_branch = parse_block_statement(tokenizer);

        auto else_symbol = tokenizer.peek_next();
        if (else_symbol.type == TOKEN_SYMBOL && else_symbol.string == "else") {
            tokenizer.read_next();
            // if_statement->if_statement.false_branch = parse_any_statement(tokenizer);
            if_statement->if_statement.false_branch = parse_block_statement(tokenizer);
        }
        return if_statement;
    }

    return nullptr;
}

Crank_Statement* parse_any_statement(Tokenizer_State& tokenizer) {
    /* exhaust all statement types to parse. */
    _debugprintf("trying to parse if\n");
    auto if_statement = parse_if_statement(tokenizer);
    if (if_statement) return if_statement;

    _debugprintf("trying to parse for\n");
    auto for_statement = parse_for_statement(tokenizer);
    if (for_statement) return for_statement;

    _debugprintf("trying to parse while\n");
    auto while_statement = parse_while_statement(tokenizer);
    if (while_statement) return while_statement;

    _debugprintf("trying to parse block\n");
    auto block_statement = parse_block_statement(tokenizer);
    if (block_statement) return block_statement;

    // NOTE: semicolon requiring statements
    _debugprintf("trying to parse return\n");
    auto return_statement = parse_return_statement(tokenizer);
    if (return_statement) {
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON && "Statement requiring semicolon");
        return return_statement;
    }

    _debugprintf("trying to parse declaration\n");
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

    _debugprintf("trying to parse expression\n");
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
            _debugprintf("Found char literal.\n");
            value.type = lookup_type("char");
            value.int_value = (int)first.valuechar;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_STRING: {
            tokenizer.read_next();
            _debugprintf("Found string literal.\n");
            value.type = lookup_type("strlit");
            // NOTE?
            value.string_value = first.string;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_NUMBERINT: {
            tokenizer.read_next();
            _debugprintf("Found number.\n");
            // I mean... I should have more specifiers to make
            // this more than just "int", although I store the full
            // 64 bit range so I guess it's okay...
            value.type = lookup_type("int");
            value.int_value = first.value32;
            return Error<Crank_Value>::okay(value);
        } break;
        case TOKEN_NUMBERFLOAT: {
            tokenizer.read_next();
            _debugprintf("Found float number.\n");
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

            _debugprintf("Found symbol.\n");
            auto next = tokenizer.peek_next();
            value.value_type = VALUE_TYPE_SYMBOL;
            if (next.type == TOKEN_COLON) {
                _debugprintf("Presuming this to be an object literal\n");
                // object literal
                /*
                 * NOTE: just keep it like this to be simple, so it's kinda like Go.
                 * Don't need a fancy parser. Need a working language.
                 *
                 * This isn't like C++ with initializers. Only for objects.
                 *
                 * NOTE: should move this to a separate procedure to reduce the size of this part
                 */
                tokenizer.read_next();
                // I don't have the ternary operator in this language so it's okay
                // to use that syntax
                assert(tokenizer.read_next().type == TOKEN_LEFT_CURLY_BRACE && "A struct literal looks like structname: {initializer}");
                value.value_type = VALUE_TYPE_LITERAL;
                value.type = lookup_type(first.string);
                _debugprintf("Trying to lookup struct : \"%.*s\"\n", unwrap_string_view(first.string));
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
                        _debugprintf("Named literal\n");
                        literal_object->type = OBJECT_LITERAL_DECL_NAMED;

                        while (tokenizer.peek_next().type != TOKEN_RIGHT_CURLY_BRACE) {
                            _debugprintf("Reading a field\n");
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
                        _debugprintf("Ordered literal\n");
                        literal_object->type = OBJECT_LITERAL_DECL_ORDERED;

                        while (tokenizer.peek_next().type != TOKEN_RIGHT_CURLY_BRACE) {
                            _debugprintf("Reading a field\n");
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
                    // NOTE: this will not output right now
                    value.value_type = VALUE_TYPE_LITERAL;
                    value.type = lookup_type("void", {}, {}, 0, 0, 1);
                    value.int_value = 0;
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
                        _debugprintf("Possible function call\n");
                        tokenizer.read_next(); 

                        value.is_function_call = true;
                        // NOTE: Ah I see. types are not looked up yet!
                        // to allow out of order declaration
                        // crank types have to patch themselves up!
                        // IE: type is probably null here.
                        // we need to look up declarations for a function definition
                        // NOTE:
                        // so add this to a list of "to resolve", and when I find a function
                        // see if we can resolve the function.
                        while (tokenizer.peek_next().type != TOKEN_RIGHT_PARENTHESIS) {
                            auto new_value = parse_expression(tokenizer);
                            assert(new_value && "Bad function param passing");
                            value.call_parameters.push_back(new_value);

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
    _debugprintf("Trying to parse for extern(%.*s)\n", unwrap_string_view(extern_token.string));
    // if I were to output native code, I'd assume standard C linkage for now.
    if (extern_token.type == TOKEN_SYMBOL && extern_token.string == "extern") {
        tokenizer.read_next();
        _debugprintf("Object is an extern declaration!\n");
        /*
         * This is technically more important for when we need to check if all functions are defined.
         */
        result.is_externally_defined = true;

        if (tokenizer.peek_next().type == TOKEN_LEFT_PARENTHESIS) {
            tokenizer.read_next();
            auto extern_linkage_name = tokenizer.read_next();
            assert(extern_linkage_name.type == TOKEN_STRING);
            result.extern_definition.linkage_name = extern_linkage_name.stringvalue;
            _debugprintf("This object is linked as \"%s\"", result.extern_definition.linkage_name.c_str());
            assert(tokenizer.read_next().type == TOKEN_RIGHT_PARENTHESIS && "Extern name param needs to be closed with a parenthesis!");
        }
    }

    auto type_entry = read_type_declaration(tokenizer);
    assert(type_entry.good && "Bad type entry?");
    _debugprintf("Checking decl type... (%s)\n", type_entry.value.name.c_str());
    /* NOTE: replace this stuff. Should be a bit faster... */
    auto type = lookup_type(
        type_entry.value.name,
        type_entry.value.array_dimensions,
        type_entry.value.call_parameters,
        type_entry.value.is_function,
        type_entry.value.is_variadic,
        type_entry.value.pointer_depth
    );
    assert(type && "Type not found! Cannot resolve!");

    result.array_dimensions = type_entry.value.array_dimensions;
    result.object_type = type; // NOTE? need to check something
    result.name = name.string;

    if (type_entry.value.is_function) {
        assert(type->is_function);
        _debugprintf("This decl is a function!\n");
        auto function_statement = parse_any_statement(tokenizer);
        if (function_statement) {
            _debugprintf("function declaration!\n");
            result.has_value = true;
            // result.value.body = function_statement;
            result.expression = new Crank_Expression;
            result.expression->type = EXPRESSION_VALUE;
            result.expression->value.body = function_statement;
        }
    } else {
        assert(!result.is_externally_defined && "Variables cannot be externally defined in this language!");
        _debugprintf("This decl is a variable!\n");
        if (tokenizer.peek_next().type == TOKEN_EQUAL) {
            /*
             * NOTE: for supporting function objects
             * I can't really have them be "expressions" not in the normal sense, because it really
             * doesn't make sense?
             *
             * and is a LOT of special casing to produce a function object in the expression parser imo...
             * at least in the way I'm doing it
             */
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
                _debugprintf("Array type checking\n");
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
    _debugprintf("Trying to read decl\n");
    while (tokenizer.peek_next().type == TOKEN_SYMBOL) { // I don't think I need to check this anymore b/c read_inline_declaration returns error but okay
        // NOTE: I should allow inner anonymous structs and unions.
        // not variants though. Those are a special case?

        auto new_member_decl = read_inline_declaration(tokenizer).value;
        assert(tokenizer.peek_next().type == TOKEN_SEMICOLON || tokenizer.peek_next().type == TOKEN_COMMA);
        tokenizer.read_next();
        // assert(new_member_decl.good && "Bad decl");
        // TODO: check for duplicates!
        if (new_member_decl.has_value) {
            printf("Sorry! No default value yet!\n");
        }
        type->members.push_back(new_member_decl);
    }
    printf("Finished reading decl! (%d members?)\n", type->members.size());
    assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
    return true;
}

bool read_union_definition(Crank_Type* type, Tokenizer_State& tokenizer) {
    assert(type->type == TYPE_UNION && "?");
    _debugprintf("Trying to read decl\n");
    while (tokenizer.peek_next().type == TOKEN_SYMBOL) { // I don't think I need to check this anymore b/c read_inline_declaration returns error but okay
        // NOTE: I should allow inner anonymous structs and unions.
        // not variants though. Those are a special case?

        auto new_member_decl = read_inline_declaration(tokenizer).value;
        assert(tokenizer.peek_next().type == TOKEN_SEMICOLON || tokenizer.peek_next().type == TOKEN_COMMA);
        tokenizer.read_next();
        assert(!new_member_decl.has_value && "Unions with default initialization are undefined behavior!");
        type->members.push_back(new_member_decl);
    }
    printf("Finished reading decl! (%d members?)\n", type->members.size());
    assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
    return true;
}

bool read_enum_definition(Crank_Type* type, Tokenizer_State& tokenizer) {
    assert(type->type == TYPE_ENUMERATION && "?");
    _debugprintf("Trying to read decl\n");

    int64_t start_counting_from = 0; // for counting values

    while (tokenizer.peek_next().type == TOKEN_SYMBOL) { // I don't think I need to check this anymore b/c read_inline_declaration returns error but okay
        // NOTE: I should allow inner anonymous structs and unions.
        // not variants though. Those are a special case?
        auto value_name = tokenizer.read_next();

        int64_t current_value = start_counting_from++;
        if (tokenizer.peek_next().type == TOKEN_EQUAL) {
            tokenizer.read_next();
            // Sorry! no expression evaluation here!
            // unless I figure out how to determine if expressions are constant!

            // found value
            auto new_value = parse_expression(tokenizer);
            // NOTE: I need to assert it is an integer.
            // NOTE: this will have undefined behavior if I can't determine it is
            // going to be an int;
            assert(new_value && "I should have an expression.");
            assert(is_expression_numeric(new_value) && "enum value should be a numeric expression!");
            assert(is_constant_expression(new_value) && "enum value should be a constant expression!");
            new_value = fold_constant_numeric_expression(new_value);

            // NOTE: have to handle case where we get a binary expression that is
            // actually an enum
            // because I have to lookup the enum and use the literal value.
            
            if (new_value->type == EXPRESSION_VALUE) {
                // definitely need a helper to get stuff working
                current_value = start_counting_from = new_value->value.int_value;
            } else {
                assert(new_value->type == EXPRESSION_BINARY && "This should be a property access, but firstly that must be binary!"); 
            }
            start_counting_from++;
        } else {
            // nothing
        }

        assert(tokenizer.peek_next().type == TOKEN_SEMICOLON || tokenizer.peek_next().type == TOKEN_COMMA);
        tokenizer.read_next();
        {
            Crank_Enum_KeyValue kv;
            kv.name = value_name.string;
            kv.value = current_value;
            _debugprintf("added enum member: %.*s\n", unwrap_string_view(value_name.string));
            type->enum_members.push_back(kv);
        }
    }
    printf("Finished reading decl! (%d members?)\n", type->enum_members.size());
    assert(tokenizer.read_next().type == TOKEN_RIGHT_CURLY_BRACE);
    return true;
}

Error<Crank_Declaration> parse_typedef(Tokenizer_State& tokenizer) {
    auto name = tokenizer.read_next();
    if (name.type != TOKEN_SYMBOL) return Error<Crank_Declaration>::fail("Typedef name is not a symbol!");
    auto separator = tokenizer.read_next();
    if (separator.type == TOKEN_SYMBOL && separator.string == "as") {
        // typedef rename
        auto type_entry = read_type_declaration(tokenizer);

        if (!type_entry.good) return Error<Crank_Declaration>::fail(type_entry.message);

        Crank_Declaration typedecl;
        typedecl.decl_type = DECL_TYPE;
        typedecl.name = name.string;
        typedecl.array_dimensions = type_entry.value.array_dimensions;

        // NOTE: if we rename based off an array type
        // there's no need to list the array dimensions.
        auto new_type = register_new_type(typedecl.name, TYPE_RENAME);
        new_type->rename_of = lookup_type(type_entry.value.name, typedecl.array_dimensions);
        typedecl.object_type = new_type;
        assert(new_type->rename_of && "Error! Typedefed type does not exist!");

        if (!typedecl.object_type) {
            // Error. Cannot resolve currently.
            // TODO: I would resolve this later but I don't have the resources to do so right now!
            return Error<Crank_Declaration>::fail("Cannot resolve type! Not known yet?");
        }
        _debugprintf("read typedef rename\n");
        // this requires a semicolon.
        assert(tokenizer.read_next().type == TOKEN_SEMICOLON);
        return Error<Crank_Declaration>::okay(typedecl);
    } else if (separator.type == TOKEN_COLON) {
        auto determiner = tokenizer.peek_next();
        if (determiner.type != TOKEN_SYMBOL) return Error<Crank_Declaration>::fail("Need symbol descriptor to discriminate!");
        tokenizer.read_next();

        Crank_Declaration typedecl;
        typedecl.decl_type = DECL_TYPE;
        typedecl.name = name.string;

        if (determiner.string == "struct") {
            typedecl.object_type = register_new_type(name.string, TYPE_RECORD);

            assert(tokenizer.read_next().type == TOKEN_LEFT_CURLY_BRACE && "Invalid start to tagged struct item");
            if (read_record_definition(typedecl.object_type, tokenizer)) {
                _debugprintf("Read new record definition\n");
                return Error<Crank_Declaration>::okay(typedecl);
            } else {
                return Error<Crank_Declaration>::fail("Failed to read record definition");
            }
            typedecl.name = name.string;
        } else if (determiner.string == "union") {
            typedecl.object_type = register_new_type(name.string, TYPE_UNION);

            assert(tokenizer.read_next().type == TOKEN_LEFT_CURLY_BRACE && "Invalid start to tagged struct item");
            if (read_union_definition(typedecl.object_type, tokenizer)) {
                _debugprintf("Read new union definition\n");
                return Error<Crank_Declaration>::okay(typedecl);
            } else {
                return Error<Crank_Declaration>::fail("Failed to read record definition");
            }
            typedecl.name = name.string;
        } else if (determiner.string == "enum") {
            typedecl.object_type = register_new_type(name.string, TYPE_ENUMERATION);

            auto enum_type = tokenizer.read_next();
            if (determiner.type != TOKEN_SYMBOL) return Error<Crank_Declaration>::fail("Need symbol descriptor to discriminate!");

            typedecl.object_type->enum_internal_type = lookup_type(enum_type.string);
            assert(is_type_integer(typedecl.object_type->enum_internal_type) && "Enums can only be tagged after integer types!");

            assert(tokenizer.read_next().type == TOKEN_LEFT_CURLY_BRACE && "Invalid start to tagged enum item");
            if (read_enum_definition(typedecl.object_type, tokenizer)) {
                _debugprintf("Read new enum definition\n");
                return Error<Crank_Declaration>::okay(typedecl);
            } else {
                return Error<Crank_Declaration>::fail("Failed to read record definition");
            }
            typedecl.name = name.string;
        } else {
            return Error<Crank_Declaration>::fail("Invalid tag for typedef!");
        }
    } else {
        return Error<Crank_Declaration>::fail("Invalid token in typedef parsing.");
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
                    } else {
                        module.decls.push_back(new_typedef);
                    }
                } else {
                    // TODO: Check for duplicates (IN THE SAME SCOPE!)
                    auto new_decl = parse_variable_decl(tokenizer);
                    if (!new_decl.good) {
                        printf("%s\n", new_decl.message);
                    } else  {
                        _debugprintf("new decl added\n");
                        if (new_decl.value.object_type->is_function && new_decl.value.name == "main") {
                            _debugprintf("Found main function!\n");
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
            case TOKEN_SEMICOLON: {
                // harmless
                tokenizer.read_next();
            } break;
            default: {
                /* TODO better error message? */
                _debugprintf("What did I read?: %.*s\n", unwrap_string_view(Token_Type_string_table[first_token.type]));
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
    register_new_type("i64",   TYPE_INTEGER64);
    register_new_type("i32",   TYPE_INTEGER32);
    register_new_type("i16",   TYPE_INTEGER16);
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

// TODO: posix/linux

void delete_file(const char* where) {
#ifdef _WIN32
    DeleteFile(where);
#else
    remove(where);
#endif
}

#include "debug_print.cc"
#include "crank_parsing_tests.cc"

int main(int argc, char** argv){
    register_default_types();

    // TODO: actual command line arguments.
#if 0
    // NOTE: should run as a test argument only
    run_all_tests();
#else
    std::vector<std::string> module_names;
    std::vector<std::string> linkage_lib_names;
    Crank_Codegen* generator = new CPlusPlusCodeGenerator();
    std::string output_name = "a";
    bool keep_cplusplus = false;

    for (int i = 1; i < argc; ++i) {
        char* current_argument = argv[i];

        if (current_argument[0] == '-') {
            if (current_argument[1] == '-') {
                char* arg_string = current_argument+2;
                if (strcmp(arg_string, "help") == 0) {
                    printf("Crank does not give help right now. Sorry!\n");
                } else if (strcmp(arg_string, "link") == 0) {
                    char* link_library = argv[++i];
                    linkage_lib_names.push_back(std::string(link_library));
                } else if (strcmp(arg_string, "dotests") == 0) {
                    printf("running tests, and quitting.\n");
                    run_all_tests();
                    return 0;
                } else if (strcmp(arg_string, "output") == 0) {
                    char* output = argv[++i];
                    output_name = std::string(output);
                } else if (strcmp(arg_string, "keepcpp") == 0) {
                    keep_cplusplus = true;
                }
            }
        } else {
            std::string filename = argv[i];
            File_Buffer buffer = File_Buffer(filename.c_str());

            if (buffer.data == nullptr) {
                printf("No file named \"%s\"!\n", filename.c_str());
                return -1;
            }

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
    }

    if (module_names.size()) {
        // generated... now compile all the modules

        // I should try to detect the tool chain that someone might have.
        std::string compile_string = "g++ ";
        compile_string += " -o " + output_name + " ";
        for (auto s : module_names) {
            compile_string += s + " ";
        }
        for (auto l : linkage_lib_names) {
            compile_string += " -l" + l + " ";
        }

        printf("calling c compiler: %s\n", compile_string.c_str());
        os_process_shell_start_and_run_synchronously((char*)compile_string.c_str());

        // delete all old files
        if (!keep_cplusplus) {
            printf("deleting temp C++ files.\n");
            for (auto s : module_names) {
                delete_file(s.c_str());
            }
        }
    }

    return 0;
#endif
}
