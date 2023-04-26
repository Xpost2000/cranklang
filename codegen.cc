// may make this a separate translation unit but I personally don't think it is necessary.

// it's really an interface.
class Crank_Codegen {
protected:
    // implement these as separate passes.
    virtual void output_preamble(FILE* output) = 0;
    virtual void output_import(FILE* output, Crank_Module* module) = 0;
    virtual void output_type(FILE* output, Crank_Type* type) = 0;

    // we're going to separate function declaration passes
    // and non function declaration passes because some language transpile
    // targets don't have out of order initialization!

    virtual void output_value(FILE* output, Crank_Value* value) = 0;
    virtual void output_unary_expression(FILE* output, Crank_Expression* expression) = 0;
    virtual void output_binary_expression(FILE* output, Crank_Expression* expression) = 0;

    virtual void output_declaration(FILE* output, Crank_Declaration* decl) = 0;
    virtual void output_function_declaration(FILE* output, Crank_Declaration* decl) = 0;

    virtual void output_statement(FILE* output, Crank_Statement* expression) = 0;

    void output_expression(FILE* output, Crank_Expression* expression) {
        if (expression) {
            switch (expression->type) {
                case EXPRESSION_VALUE: {
                    output_value(output, &expression->value);
                } break;
                case EXPRESSION_UNARY: {
                    output_unary_expression(output, expression);
                } break;
                case EXPRESSION_BINARY: {
                    output_binary_expression(output, expression);
                } break;
            }
        }
    }

    virtual void output_entry_point(FILE* output) {}

public:
    Crank_Codegen() {}
    ~Crank_Codegen() {}
    void output_module(Crank_Module& module) {
        auto module_file_name = get_module_compiled_name(module);
        FILE* file = fopen(module_file_name.c_str(), "wb+");

        printf("Outputting preamble\n");
        output_preamble(file);

        printf("Outputting imports!\n");
        for (auto& import : module.imports) {
            output_import(file, import);
        }

        printf("Outputting declarations!\n");
        for (auto& decl : module.decls) {
            if (decl.name == "main") {
                decl.name = "crank_mainpoint_entry";
            }
            output_declaration(file, &decl);
        }

        for (auto& decl : module.decls) {
            if (decl.object_type->is_function) {
                printf("outputting function decl!\n");

                if (decl.name == "main") {
                    decl.name = "crank_mainpoint_entry";
                }
                output_function_declaration(file, &decl);
            }
        }

        if (module.has_main) {
            output_entry_point(file);
        }

        fclose(file);
    }

    virtual std::string get_module_compiled_name(Crank_Module& module) = 0;
};

/*
  NOTE: function expressions need an extra step,
  but this will be handled later...

  (IE: anonymous functions will have to be added to declarations posthumously.)
*/
class Crank_Codegen_Partial_CStyleLanguage : public Crank_Codegen {
public:
    Crank_Codegen_Partial_CStyleLanguage() {}
    ~Crank_Codegen_Partial_CStyleLanguage() {}
protected:
    void output_value(FILE* output, Crank_Value* value) {
        if (value->value_type == VALUE_TYPE_SYMBOL) {
            fprintf(output, "%s", value->symbol_name.c_str());
            if (value->is_function_call) {
                fprintf(output, "(");
                for (int i = 0; i < value->call_parameters.size(); ++i) {
                    output_expression(output, value->call_parameters[i]);
                    if (i+1 >= value->call_parameters.size()) {
                    } else {
                        fprintf(output, ", ");
                    }
                }
                fprintf(output, ")");
            }
        } else {
            // NOTE: TODO ON ARRAYS
            // AND FUNCTIONS?
            
            // Object Literal;
            if (value->array_elements.size()) {
                fprintf(output, "{");
                for (int i = 0; i < value->array_elements.size(); ++i) {
                    // output_value(output, &value->array_elements[i]);
                    output_expression(output, value->array_elements[i]);
                    if (i+1 >= value->array_elements.size()) {
                        
                    } else {
                        fprintf(output, ", ");
                    }
                }
                fprintf(output, "}");
            } else {
                auto typeof_object = value->type;
                switch (typeof_object->type) {
                    case TYPE_NUMERIC: {
                        // ???
                    } break;
                    case TYPE_BOOLEAN: {
                        if (value->int_value) {
                            fprintf(output, "true");
                        } else {
                            fprintf(output, "true");
                        }
                    } break;
                    case TYPE_INTEGER: {
                        fprintf(output, "%d", value->int_value);
                    } break;
                    case TYPE_FLOAT: {
                        fprintf(output, "%f", value->float_value);
                    } break;
                    case TYPE_CHAR: {
                        fprintf(output, "(char)%d", value->int_value);
                    } break;
                    case TYPE_STRINGLITERAL: {
                        fprintf(output, "\"%s\"", value->string_value.c_str());
                    } break;
                    case TYPE_RECORD: {
                        // TODO: parse
                    } break;
                    case TYPE_VOID: {
                    
                    } break;
                }
            }
        }
    }
    void output_unary_expression(FILE* output, Crank_Expression* expression) {
        if (expression) {
            fprintf(output, "%s", Crank_Expression_Operator_string_table[expression->operation]);
            output_expression(output, expression->unary.value);
        }
    }
    void output_binary_expression(FILE* output, Crank_Expression* expression) {
        if (expression->operation == OPERATOR_ARRAY_INDEX) {
            output_expression(output, expression->binary.first);
            fprintf(output, "[");
            output_expression(output, expression->binary.second);
            fprintf(output, "]");
        } else if (expression->operation == OPERATOR_PROPERTY_ACCESS) {
            output_expression(output, expression->binary.first);
            fprintf(output, ".", Crank_Expression_Operator_string_table[expression->operation]);
            output_expression(output, expression->binary.second);
        } else {
            output_expression(output, expression->binary.first);
            fprintf(output, "%s", Crank_Expression_Operator_string_table[expression->operation]);
            output_expression(output, expression->binary.second);
        }
    }
};
