void _debug_print_expression_tree(Crank_Expression* root);
void _debug_print_crank_value(Crank_Value value);
void _debug_print_statement(Crank_Statement* statement);

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
                } else if (value.type->type == TYPE_CHAR) {
                    printf("(char \"%c\") ", value.int_value);
                } else {
                    printf("(unprintable) ");
                    if (value.type->type == TYPE_RECORD) {
                        printf("(object-type) ");
                        auto object_literal = value.literal_value;
                        if (object_literal->type == OBJECT_LITERAL_DECL_ORDERED) {
                            for (int i = 0; i < object_literal->expressions.size(); ++i) {
                                printf("(idx %d ", i);
                                // _debug_print_crank_value(object_literal->values[i]);
                                _debug_print_expression_tree(object_literal->expressions[i]);
                                printf(") ");
                            }
                        } else if (object_literal->type == OBJECT_LITERAL_DECL_NAMED) {
                            for (int i = 0; i < object_literal->named_values.size(); ++i) {
                                printf("(idx %s ", object_literal->named_values[i].name.c_str());
                                // _debug_print_crank_value(object_literal->named_values[i].value);
                                _debug_print_expression_tree(object_literal->named_values[i].expression);
                                printf(") ");
                            }
                        }
                    }
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
                printf("(%s ", Crank_Expression_Operator_string_table[root->operation]);
                if (root->binary.first)
                    _debug_print_expression_tree(root->binary.first);
                else printf("[nil first] ");
                if (root->binary.second) _debug_print_expression_tree(root->binary.second);
                else printf("[nil second] ");
                printf(") ");
            } break;
        }
}

void _debug_print_statement(Crank_Statement* statement) {
    printf("((%s) ", Crank_Statement_Type_string_table[statement->type]);
    switch (statement->type) {
        case STATEMENT_BLOCK: {
            printf("\n");
            for (auto& inner_statement : statement->block_statement.body) {
                _debug_print_statement(inner_statement);
                printf("\n");
            }
            printf("\n");
        } break;
        case STATEMENT_IF: {
            _debug_print_expression_tree(statement->if_statement.condition);
            printf("\n");
            if (statement->if_statement.true_branch) {
                _debug_print_statement(statement->if_statement.true_branch);
            }
            printf("\n");
            if (statement->if_statement.false_branch) {
                _debug_print_statement(statement->if_statement.false_branch);
            }
            printf("\n");
        } break;
        case STATEMENT_WHILE: {
            _debug_print_expression_tree(statement->while_statement.condition);
            printf("\n");
            if (statement->while_statement.action) {
                _debug_print_statement(statement->while_statement.action);
            }
        } break;
        case STATEMENT_EXPRESSION: {
            assert(statement->expression_statement.expression);
            _debug_print_expression_tree(statement->expression_statement.expression);
        } break;
        case STATEMENT_RETURN: {
            _debug_print_expression_tree(statement->return_statement.result);
        } break;
    }
    printf(") ");
}
