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
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);
        assert(t && "This should at least pass parsing at minimum.");
        _debug_print_statement(t);
    }
}

void parse_object_literal() {
    // fields are not checked yet.
    auto record_type = register_new_type("Record", TYPE_RECORD);

    // type 1:
    {
        char* test_parse =
            R"(
StructType { 1, 2, 3, 4, 5, 6, };
)";
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);
        assert(t && "This should at least pass parsing at minimum.");
        _debug_print_statement(t);
    }
    printf("\n");
    // type 2:
    {
        char* test_parse =
            R"(
StructType { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6, };
)";
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);
        assert(t && "This should at least pass parsing at minimum.");
        _debug_print_statement(t);
    }
}

void parse_declaration() {
    // TODO: need to check for what happens when I parse a declaration
    // with an expression, since that's the other thing that's kinda busted right now
}

void run_all_tests() {
    parse_statement_boolean();
    parse_object_literal();
    parse_declaration();
}
