/*
 * Test codegen.
 *
 * this is kind of trash.
 */

class CPlusPlusCodeGenerator :
    public Crank_Codegen_Partial_CStyleLanguage
{
public:
    CPlusPlusCodeGenerator() {}
    ~CPlusPlusCodeGenerator() {}

    std::string get_module_compiled_name(Crank_Module& module) {
        return "crank_" + module.module_name + ".cpp";
    }
protected:
    void output_preamble(FILE* output) {
        /**
           NOTE: this is a separate but identical file. Mostly since I don't want
           to add additional build steps.
         **/
        auto crank_preamble_cpp =
            #include "crank-cpp-runtime/crank_preamble_include_version.h"
            ;

        fprintf(output, "%s", crank_preamble_cpp);
    }
    
    void output_import(FILE* output, Crank_Module* module) {
        // no imports yet! nothing!
    }

    // idk how to output function pointers quite yet!
    // so that's undefined behavior!
    void output_type(FILE* output, Crank_Type* type) {
        printf("output type\n");
        while (type->rename_of) type = type->rename_of; // just in case anything weird happens...
        if (type->type == TYPE_STRINGLITERAL) {
            fprintf(output, "std::string ");
        } else {
            fprintf(output, "%s ", type->name.c_str());
        }
        if (type->is_function) {
            // idk;
        }
    }

    // STUPID TODO: REMOVE OR FIX
    void output_function_param_item(FILE* output, Crank_Declaration* decl) {
        if (decl->decl_type == DECL_OBJECT) {
            printf("outputting object\n");
            output_type(output, decl->object_type);
            fprintf(output, " %s", decl->name.c_str());
            if (decl->object_type->array_dimensions.size()) {
                for (auto dimension : decl->object_type->array_dimensions) {
                    fprintf(output, "[");
                    if (dimension != -1) {
                        fprintf(output, "%d", dimension);
                    }
                    fprintf(output, "]");
                }
            }

            if (decl->object_type->is_function) {
                // LOLLLLL
                fprintf(output, "(");
                fprintf(output, ")");
            } else {
                if (decl->has_value) {
                    fprintf(output, " = ");
                    // output_value(output, &decl->value);
                    output_expression(output, decl->expression);
                }
            }
        } else {
            printf("outputting typedef\n");
            if (decl->object_type->type == TYPE_RENAME) {
                fprintf(output, "typedef %s ", decl->name.c_str());
                output_type(output, decl->object_type->rename_of);
            } else {
                fprintf(output, "struct %s {", decl->name.c_str());
                for (auto& member : decl->object_type->members) {
                    output_declaration(output, &member);
                }
                fprintf(output, "};", decl->name.c_str());
            }
        }
    }

    void output_declaration(FILE* output, Crank_Declaration* decl) {
        printf("outputting decl\n");
        if (decl->decl_type == DECL_OBJECT) {
            printf("outputting object\n");
            output_type(output, decl->object_type);
            fprintf(output, " %s", decl->name.c_str());
            if (decl->object_type->array_dimensions.size()) {
                for (auto dimension : decl->object_type->array_dimensions) {
                    fprintf(output, "[");
                    if (dimension != -1) {
                        fprintf(output, "%d", dimension);
                    }
                    fprintf(output, "]");
                }
            }

            if (decl->object_type->is_function) {
                fprintf(output, "(");
                for (int i = 0; i < decl->object_type->call_parameters.size(); ++i) {
                    output_function_param_item(output, &decl->object_type->call_parameters[i]);
                    if (i+1 >= decl->object_type->call_parameters.size()) {}
                    else {
                        fprintf(output, ", ");
                    }
                }
                fprintf(output, ")");
            } else {
                if (decl->has_value) {
                    fprintf(output, " = ");
                    // output_value(output, &decl->value);
                    output_expression(output, decl->expression);
                } else {
                    fprintf(output, " = {}");
                }
            }
        } else {
            printf("outputting typedef\n");
            if (decl->object_type->type == TYPE_RENAME) {
                fprintf(output, "typedef %s ", decl->name.c_str());
                output_type(output, decl->object_type->rename_of);
            } else {
                fprintf(output, "struct %s {", decl->name.c_str());
                for (auto& member : decl->object_type->members) {
                    output_declaration(output, &member);
                }
                fprintf(output, "};", decl->name.c_str());
            }
        }
        fprintf(output, ";");
        fprintf(output, "\n");
    }

    void output_function_declaration(FILE* output, Crank_Declaration* decl) {
        assert(decl->decl_type == DECL_OBJECT && "???");
        output_type(output, decl->object_type);
        printf("I am %s\n", decl->name.c_str());
        fprintf(output, " %s", decl->name.c_str());

        // array types are confusing for functions! LOL
        // NOTE: investigate what happens
        if (decl->object_type->array_dimensions.size()) {
            for (auto dimension : decl->object_type->array_dimensions) {
                fprintf(output, "[");
                if (dimension != -1) {
                    fprintf(output, "%d", dimension);
                }
                fprintf(output, "]");
            }
        }

        printf("output param list\n");
        fprintf(output, "(");
        for (int i = 0; i < decl->object_type->call_parameters.size(); ++i) {
            output_function_param_item(output, &decl->object_type->call_parameters[i]);
            if (i+1 >= decl->object_type->call_parameters.size()) {}
            else {
                fprintf(output, ", ");
            }
        }
        fprintf(output, ")\n");

        printf("output body?\n");
        {
            if (decl->expression) {
                auto body = decl->expression->value.body;
                if (body) {
                    // assert(body && "this should have a body");
                    if (body->type != STATEMENT_BLOCK) {
                        fprintf(output, "{\n");
                        if (body->type == STATEMENT_EXPRESSION) {
                            fprintf(output, "return ");
                        }
                    }
                    output_statement(output, body);
                    if (body->type != STATEMENT_BLOCK) {
                        if (body->type == STATEMENT_EXPRESSION) {
                            fprintf(output, ";\n");
                        }
                        fprintf(output, "}\n");
                    }
                }
            } else {
                fprintf(output, ";\n");
            }
        }
        printf("finish output body?\n");
    }

    void output_statement(FILE* output, Crank_Statement* statement) {
        switch (statement->type) {
            case STATEMENT_BLOCK: {
                fprintf(output, "{\n");
                for (auto& inner_statement : statement->block_statement.body) {
                    output_statement(output, inner_statement);
                }
                fprintf(output, "}\n");
            } break;
            case STATEMENT_IF: {
                fprintf(output, "if (");
                output_expression(output, statement->if_statement.condition);
                fprintf(output, ") ");
                fprintf(output, "\n");
                if (statement->if_statement.true_branch) {
                    output_statement(output, statement->if_statement.true_branch);
                }
                fprintf(output, "\n");
                if (statement->if_statement.false_branch) {
                    fprintf(output, " else ");
                    output_statement(output, statement->if_statement.false_branch);
                }
                fprintf(output, "\n");
            } break;
            case STATEMENT_WHILE: {
                fprintf(output, "while (");
                output_expression(output, statement->while_statement.condition);
                fprintf(output, ") ");
                fprintf(output, "\n");
                if (statement->while_statement.action) {
                    output_statement(output, statement->while_statement.action);
                }
                fprintf(output, "\n");
            } break;
            case STATEMENT_EXPRESSION: {
                assert(statement->expression_statement.expression);
                output_expression(output, statement->expression_statement.expression);
                fprintf(output, ";");
            } break;
            case STATEMENT_DECLARATION: {
                output_declaration(output, statement->declaration_statement.declaration);
                fprintf(output, ";");
            } break;
            case STATEMENT_RETURN: {
                fprintf(output, "return ");
                output_expression(output, statement->return_statement.result);
                fprintf(output, ";\n");
            } break;
        }
        printf("\n");
    }

    // MAIN should have specific signature but whatever!
    void output_entry_point(FILE* output) {
        /** for C++ **/
#define AS_STRING
        auto crank_main_point_entry_cpp =
            #include "crank-cpp-runtime/crank_main_point_entry.cpp"
            ;
        fprintf(output, "%s", crank_main_point_entry_cpp);
    }
};
