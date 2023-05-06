// may make this a separate translation unit but I personally don't think it is necessary.

// it's really an interface.
class Crank_Codegen {
protected:
    // implement these as separate passes.
    virtual void output_preamble(Crank_Module& current_module, FILE* output) = 0;
    virtual void output_import(Crank_Module& current_module, FILE* output, Crank_Module* module) = 0;
    virtual void output_type(Crank_Module& current_module, FILE* output, Crank_Type* type) = 0;

    // we're going to separate function declaration passes
    // and non function declaration passes because some language transpile
    // targets don't have out of order initialization!

    virtual void output_value(Crank_Module& current_module, FILE* output, Crank_Value* value) = 0;
    virtual void output_unary_expression(Crank_Module& current_module, FILE* output, Crank_Expression* expression) = 0;
    virtual void output_binary_expression(Crank_Module& current_module, FILE* output, Crank_Expression* expression) = 0;

    virtual void output_declaration(Crank_Module& current_module, FILE* output, Crank_Declaration* decl) = 0;
    virtual void output_function_declaration(Crank_Module& current_module, FILE* output, Crank_Declaration* decl) = 0;

    virtual void output_statement(Crank_Module& current_module, FILE* output, Crank_Statement* expression) = 0;

    void output_expression(Crank_Module& current_module, FILE* output, Crank_Expression* expression) {
        if (expression) {
            switch (expression->type) {
                case EXPRESSION_VALUE: {
                    output_value(current_module, output, &expression->value);
                } break;
                case EXPRESSION_UNARY: {
                    output_unary_expression(current_module, output, expression);
                } break;
                case EXPRESSION_BINARY: {
                    output_binary_expression(current_module, output, expression);
                } break;
            }
        }
    }

    virtual void output_entry_point(Crank_Module& current_module, FILE* output) {}

public:
    Crank_Codegen() {}
    ~Crank_Codegen() {}

    // note keep set of modules.
    void output_module(Crank_Module& module) {
        auto module_file_name = get_module_compiled_name(module);
        FILE* file = fopen(module_file_name.c_str(), "wb+");

        output_preamble(module, file);

        for (auto& import : module.imports) {
            output_import(module, file, import);
        }

        for (auto& decl : module.decls) {
            if (decl.name == "main") {
                decl.name = "crank_mainpoint_entry";
            }
            output_declaration(module, file, &decl);
        }

        for (auto& decl : module.decls) {
            if (decl.object_type->is_function) {
                output_function_declaration(module, file, &decl);
            }
        }

        if (module.has_main) {
            output_entry_point(module, file);
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
    void output_value(Crank_Module& current_module, FILE* output, Crank_Value* value) {
        if (value->value_type == VALUE_TYPE_SYMBOL) {
            // Check for external names to remap
            if (value->is_function_call) {
                // lookup it's definition.
                auto decl = crank_module_find_function_decl(current_module, (char*)value->symbol_name.c_str());
                if (!decl) {
                    printf("Function \"%s\" does not have a declaration anywhere?\n", value->symbol_name.c_str());
                    assert(decl && "No function declaration!");
                }
                if (decl->is_externally_defined && decl->extern_definition.linkage_name != "") {
                    fprintf(output, "%s", decl->extern_definition.linkage_name.c_str());
                } else {
                    fprintf(output, "%s", value->symbol_name.c_str());
                }
            } else {
                fprintf(output, "%s", value->symbol_name.c_str());
            }

            if (value->is_function_call) {
                
                fprintf(output, "(");
                for (int i = 0; i < value->call_parameters.size(); ++i) {
                    output_expression(current_module, output, value->call_parameters[i]);
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
            // AND POINTERS?
            
            // Object Literal;
            if (value->array_elements.size()) {
                fprintf(output, "{");
                for (int i = 0; i < value->array_elements.size(); ++i) {
                    output_expression(current_module, output, value->array_elements[i]);
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
                            fprintf(output, "false");
                        }
                    } break;
                    case TYPE_UNSIGNEDINTEGER8:
                    case TYPE_UNSIGNEDINTEGER16:
                    case TYPE_UNSIGNEDINTEGER32:
                    case TYPE_UNSIGNEDINTEGER64:
                    {
                        fprintf(output, "%u", value->uint_value);
                    } break;
                    case TYPE_INTEGER8:
                    case TYPE_INTEGER16:
                    case TYPE_INTEGER32:
                    case TYPE_INTEGER64:
                    {
                        fprintf(output, "%d", value->int_value);
                    } break;
                    case TYPE_FLOAT:
                    case TYPE_DOUBLE:
                    {
                        fprintf(output, "%f", value->float_value);
                    } break;
                    case TYPE_STRINGLITERAL: {
                        fprintf(output, "\"%s\"", value->string_value.c_str());
                    } break;
                    case TYPE_RECORD: {
                        // TODO: parse
                        // this is parsed but we need to figure something out.
                    } break;
                    case TYPE_VOID: {
                    
                    } break;
                }
            }
        }
    }

    void output_unary_expression(Crank_Module& current_module, FILE* output, Crank_Expression* expression) {
        if (expression) {
            fprintf(output, "(%s", Crank_Expression_Operator_string_table[expression->operation]);
            output_expression(current_module, output, expression->unary.value);
            fprintf(output, ")");
        }
    }

    void output_binary_expression(Crank_Module& current_module, FILE* output, Crank_Expression* expression) {
        if (expression->operation == OPERATOR_ARRAY_INDEX) {
            output_expression(current_module, output, expression->binary.first);
            fprintf(output, "[");
            output_expression(current_module, output, expression->binary.second);
            fprintf(output, "]");
        } else if (expression->operation == OPERATOR_PROPERTY_ACCESS) {
            output_expression(current_module, output, expression->binary.first);
            fprintf(output, ".");
            output_expression(current_module, output, expression->binary.second);
        } else {
            fprintf(output, "(");
            output_expression(current_module, output, expression->binary.first);
            fprintf(output, "%s", Crank_Expression_Operator_string_table[expression->operation]);
            output_expression(current_module, output, expression->binary.second);
            fprintf(output, ")");
        }
    }
};
