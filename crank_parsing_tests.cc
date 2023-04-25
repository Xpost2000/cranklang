/*
 * this isn't a formal unit test, this is just
 * a bunch of functions.
 *
 * Until I really have proper equality checking facilities
 * I still have to basically check these manually.
 */

// I am expecting a boolean expression statement.
void parse_statement_boolean() {
    {
        char* test_parse =
            R"(
true;
)";
        // printf("Parsing: %s\n", test_parse);
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);

        assert(t && "This should at least pass parsing at minimum.");
        _debug_print_statement(t);
    }
    printf("\n");
    {
        char* test_parse =
            R"(
false;
)";
        // printf("Parsing: %s\n", test_parse);
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);
        // auto t = parse_block_statement(tokenizer);

        assert(t && "This should at least pass parsing at minimum.");
        _debug_print_statement(t);
    }
}

void parse_object_literal() {
    
}

void parse_declaration() {
    
}

void run_all_tests() {
    parse_statement_boolean();
    parse_object_literal();
    parse_declaration();
}
