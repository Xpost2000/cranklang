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
    void output_preamble(Crank_Module& current_module, FILE* output) {
        /**
           NOTE: this is a separate but identical file. Mostly since I don't want
           to add additional build steps.
         **/
        auto crank_preamble_cpp =
            #include "crank-cpp-runtime/crank_preamble_include_version.h"
            ;

        fprintf(output, "%s", crank_preamble_cpp);
    }
    
    void output_import(Crank_Module& current_module, FILE* output, Crank_Module* module) {
        // no imports yet! nothing!
    }

    // idk how to output function pointers quite yet!
    // so that's undefined behavior!
    void output_type(Crank_Module& current_module, FILE* output, Crank_Type* type) {
        printf("output type\n");
        while (type->rename_of) type = type->rename_of; // just in case anything weird happens...
        if (type->type == TYPE_STRINGLITERAL) {
            fprintf(output, "std::string ");
        } else {
            fprintf(output, "%s ", type->name.c_str());
        }

        // it's a bit of a pain in the ass for variables lol in C++.
        // need to think about that one.
        if (type->is_function) {
            // idk;
        }

        for (int ptr = 0; ptr < type->pointer_depth; ++ptr) {
            fprintf(output, "*");
        }
    }

    // STUPID TODO: REMOVE OR FIX
    void output_function_param_item(Crank_Module& current_module, FILE* output, Crank_Declaration* decl) {
        if (decl->decl_type == DECL_OBJECT) {
            printf("outputting object\n");
            output_type(current_module, output, decl->object_type);
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
                    output_expression(current_module, output, decl->expression);
                }
            }
        } else {
            printf("outputting typedef\n");
            if (decl->object_type->type == TYPE_RENAME) {
                fprintf(output, "typedef %s ", decl->name.c_str());
                output_type(current_module, output, decl->object_type->rename_of);
            } else {
                fprintf(output, "struct %s {", decl->name.c_str());
                for (auto& member : decl->object_type->members) {
                    output_declaration(current_module, output, &member);
                }
                fprintf(output, "};", decl->name.c_str());
            }
        }
    }

    void output_declaration(Crank_Module& current_module, FILE* output, Crank_Declaration* decl) {
        printf("outputting decl\n");
        if (decl->is_externally_defined) {
            fprintf(output, "extern \"C\" {\n");
        }

        if (decl->decl_type == DECL_OBJECT) {
            printf("outputting object\n");

            output_type(current_module, output, decl->object_type);

            if (decl->is_externally_defined && decl->extern_definition.linkage_name != "") {
                fprintf(output, " %s", decl->extern_definition.linkage_name.c_str());
            } else {
                fprintf(output, " %s", decl->name.c_str());
            }

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
                    output_function_param_item(current_module, output, &decl->object_type->call_parameters[i]);
                    if (i+1 >= decl->object_type->call_parameters.size()) {}
                    else {
                        fprintf(output, ", ");
                    }
                }
                fprintf(output, ")");
            } else {
                if (decl->has_value) {
                    fprintf(output, " = ");
                    output_expression(current_module, output, decl->expression);
                } else {
                    fprintf(output, " = {}");
                }
            }
        } else {
            printf("outputting typedef\n");
            switch (decl->object_type->type) {
                case TYPE_RENAME: { // typedef of the normal kind
                    fprintf(output, "typedef ");
                    output_type(current_module, output, decl->object_type->rename_of);
                    fprintf(output, " %s", decl->name.c_str());
                } break;
                case TYPE_RECORD: { // struct
                    fprintf(output, "struct %s { // test name\n", decl->name.c_str());
                    for (auto& member : decl->object_type->members) {
                        output_declaration(current_module, output, &member);
                    }
                    fprintf(output, "};", decl->name.c_str());
                } break;

                default: {
                    assert(!"unsupported codegen");
                } break;
                // case TYPE_UNION: { // union
                    
                // } break;
                // case TYPE_ENUMERATION: { // enum
                    
                // } break;
                    
            }
        }
        fprintf(output, ";");
        if (decl->is_externally_defined) {
            fprintf(output, "}; // end extern \"C\" \n");
        }
        fprintf(output, "\n");
    }

    void output_function_declaration(Crank_Module& current_module, FILE* output, Crank_Declaration* decl) {
        if (decl->expression) {
            assert(!decl->is_externally_defined && "An externally defined function should not have a definition");
            assert(decl->decl_type == DECL_OBJECT && "???");
            output_type(current_module, output, decl->object_type);
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
                output_function_param_item(current_module, output, &decl->object_type->call_parameters[i]);
                if (i+1 >= decl->object_type->call_parameters.size()) {}
                else {
                    fprintf(output, ", ");
                }
            }
            fprintf(output, ")\n");

            printf("output body?\n");
            {
                auto body = decl->expression->value.body;
                if (body) {
                    // assert(body && "this should have a body");
                    if (body->type != STATEMENT_BLOCK) {
                        fprintf(output, "{\n");
                        if (body->type == STATEMENT_EXPRESSION) {
                            fprintf(output, "return ");
                        }
                    }
                    output_statement(current_module, output, body);
                    if (body->type != STATEMENT_BLOCK) {
                        if (body->type == STATEMENT_EXPRESSION) {
                            fprintf(output, ";\n");
                        }
                        fprintf(output, "}\n");
                    }
                }
            }
            printf("finish output body?\n");
        }
    }

    void output_statement(Crank_Module& current_module, FILE* output, Crank_Statement* statement) {
        switch (statement->type) {
            case STATEMENT_BLOCK: {
                fprintf(output, "{\n");
                for (auto& inner_statement : statement->block_statement.body) {
                    output_statement(current_module, output, inner_statement);
                }
                fprintf(output, "}\n");
            } break;
            case STATEMENT_IF: {
                fprintf(output, "if (");
                output_expression(current_module, output, statement->if_statement.condition);
                fprintf(output, ") ");
                fprintf(output, "\n");
                if (statement->if_statement.true_branch) {
                    output_statement(current_module, output, statement->if_statement.true_branch);
                }
                fprintf(output, "\n");
                if (statement->if_statement.false_branch) {
                    fprintf(output, " else ");
                    output_statement(current_module, output, statement->if_statement.false_branch);
                }
                fprintf(output, "\n");
            } break;
            case STATEMENT_WHILE: {
                fprintf(output, "while (");
                output_expression(current_module, output, statement->while_statement.condition);
                fprintf(output, ") ");
                fprintf(output, "\n");
                if (statement->while_statement.action) {
                    output_statement(current_module, output, statement->while_statement.action);
                }
                fprintf(output, "\n");
            } break;
            case STATEMENT_EXPRESSION: {
                assert(statement->expression_statement.expression);
                output_expression(current_module, output, statement->expression_statement.expression);
                fprintf(output, ";");
            } break;
            case STATEMENT_DECLARATION: {
                output_declaration(current_module, output, statement->declaration_statement.declaration);
                fprintf(output, ";");
            } break;
            case STATEMENT_RETURN: {
                fprintf(output, "return ");
                output_expression(current_module, output, statement->return_statement.result);
                fprintf(output, ";\n");
            } break;
        }
        printf("\n");
    }

    void output_value(Crank_Module& current_module, FILE* output, Crank_Value* value) {
        bool overrode_behavior = false;
        if (value->value_type == VALUE_TYPE_LITERAL) {
            auto typeof_object = value->type;

            if (typeof_object->type == TYPE_STRINGLITERAL) {
                // C++ is too permissive lol. So I need to figure this one out.
                // TODO: write a crank string runtime type
                // fprintf(output, "std::string(\"%s\")", value->string_value.c_str());
                // overrode_behavior = true;
            } else if (typeof_object->type == TYPE_RECORD) {
                /* record literals
                   NOTE/TODO: I don't really trust C++ to produce these correctly so I should
                   technically always make a function that generates an object...
                   Especially since C++11 doesn't promise designated initializers to work correctly.
                   but this leads to a special case in the initializer. Also it means I cannot use

                   for the sake of simplicity. I'll output it the same way that it's read-in for now.
                   I will think of making "constructor" objects later
                */
                auto& literal_value = value->literal_value;
                assert(literal_value && "This should not be possible? Null literal value");
                assert(literal_value->named_values.size() <= typeof_object->members.size() &&
                       literal_value->expressions.size() <= typeof_object->members.size() &&
                       "initializer is too large!");
                fprintf(output, "{\n");

                // would rather just put this through the constructor but okay
                if (literal_value->type == OBJECT_LITERAL_DECL_ORDERED) {
                    for (auto& expression : literal_value->expressions) {
                        output_expression(current_module, output, expression);
                        fprintf(output, ",");
                    }
                } else if (literal_value->type == OBJECT_LITERAL_DECL_NAMED) {
                    for (auto& named_value : literal_value->named_values) {
                        fprintf(output, "%s: ", named_value.name.c_str());
                        output_expression(current_module, output, named_value.expression);
                        fprintf(output, ",");
                    }
                }
                // for (typeof_object->)
                fprintf(output, "}\n");
                overrode_behavior = true;
            }
        }

        if (!overrode_behavior) {
            Crank_Codegen_Partial_CStyleLanguage::output_value(current_module, output, value);
        }
    }

    // MAIN should have specific signature but whatever!
    void output_entry_point(Crank_Module& current_module, FILE* output) {
        /** for C++ **/
        auto crank_main_point_entry_cpp =
            #include "crank-cpp-runtime/crank_main_point_entry.cpp"
            ;
        fprintf(output, "%s", crank_main_point_entry_cpp);
    }
};
