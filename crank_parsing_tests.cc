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
Record { 1, 2, 3, 4, 5, 6, };
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
Record { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6, };
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

// this is arguably the most important test
// since parsing is more obvious, if it's broken.
void typecheck_matching() {
    auto record_type1 = register_new_type("FictionalType1", TYPE_RECORD);
    auto record_type2 = register_new_type("FictionalType2", TYPE_RECORD);

    {
        printf("Check sanity\n");
        assert(crank_type_match(lookup_type("int"), lookup_type("int")) && "Cannot check type with itself?");
        assert(crank_type_match(lookup_type("bool"), lookup_type("bool")) && "Cannot check type with itself?");
        assert(!crank_type_match(lookup_type("bool"), lookup_type("int")) && "This should not match.");
        assert(!crank_type_match(lookup_type("int"), lookup_type("strlit")) && "This should not match.");
        assert(!crank_type_match(lookup_type("FictionalType1"), lookup_type("FictionalType2")) && "This should not match.");
    }
    /*
      NOTE: these are the basic rules I am trying to obey.
        abc: int(array: int[]) {
        }

        def: int(array: int[4]) {
        }

        main: int(argc: int, arguments: strlit[]) {
        array2: int[4];
        array: int[];

        abc(array2); // GOOD
        abc(array); // GOOD

        def(array2); // GOOD
        def(array); // BAD
        }
     */
    {
        auto new_type = register_new_type("i32", TYPE_RENAME);
        new_type->rename_of = lookup_type("int");

        auto new_type2 = register_new_type("i32_array", TYPE_RENAME);
        new_type2->rename_of = lookup_type("int", {-1});

        auto new_type3 = register_new_type("i32_array16", TYPE_RENAME);
        new_type3->rename_of = lookup_type("int", {16});

        printf("Typedef checking\n");

        assert(crank_type_match(new_type, lookup_type("int")) && "Typedef should be safe.");
        assert(crank_type_match(new_type2, lookup_type("int", {-1})) && "Typedef of array should be safe");
        assert(crank_type_match(new_type3, lookup_type("int", {16})) && "Typedef of array should be safe");
        assert(!crank_type_match(new_type3, lookup_type("int", {10})) && "Typedef array should not match here");
    }
    {
        printf("Array type checking\n");
        assert(crank_type_match(lookup_type("int", {-1}), lookup_type("int", {-1})) && "Array types should be okay.");
        assert(crank_type_match(lookup_type("int", {2}), lookup_type("int", {2})) && "Array types should be okay.");
        assert(crank_type_match(lookup_type("int", {2, 2}), lookup_type("int", {2, 2})) && "Array types should be okay.");
        assert(!crank_type_match(lookup_type("int", {1}), lookup_type("int", {2})) && "These should not match");
        assert(!crank_type_match(lookup_type("int", {2, 2}), lookup_type("int", {1, 2})) && "These should not match");
        assert(!crank_type_match(lookup_type("int", {2, -1}), lookup_type("int", {1, -1})) && "These should not match");
        assert(!crank_type_match(lookup_type("int"), lookup_type("int", {-1})) && "These should not match.");
        assert(crank_type_match(lookup_type("int", {-1, -1, -1}), lookup_type("int", {-1, -1, -1})) && "Array types should be okay.");
        assert(!crank_type_match(lookup_type("int", {-1, -1, -1}), lookup_type("int", {-1, -1})) && "These should not match.");

        // NOTE:
        // I know this brings into question
        // what happens if a function taking an array reference/pointer decides to push or anything
        // in that case, I'll enforce a runtime check.
        // This language isn't meant to be "efficient".
        assert(crank_type_match(lookup_type("int", {4}), lookup_type("int", {-1})) && "This is legal because I can promote the type");
        assert(!crank_type_match(lookup_type("int", {-1}), lookup_type("int", {4})) && "This is illegal, because a fixed size array is inferior to a flexible one");
    }

    {
        printf("Function type checking.\n");
        // TODO
    }
}

void run_all_tests() {
    typecheck_matching();
    parse_statement_boolean();
    parse_object_literal();
    parse_declaration();
}
