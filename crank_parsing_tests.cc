/*
 * this isn't a formal unit test, this is just
 * a bunch of functions.
 *
 * Until I really have proper equality checking facilities
 * I still have to basically check these manually.
 */

// I am expecting a boolean expression statement.
void parse_statements() {
    // this should be an empty statement
    {
        char* test_parse =
            R"(
;
)";
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);
        assert(!t && "This is an empty statement");
    }
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
    printf("\n");
#if 0
    {
        char* test_parse =
            R"(
switch 4 {
   case 5:
   case 6:
   case 7:
     break;
   default:
   break;
}
)";
        printf("Parsing: %s\n", test_parse);
        Tokenizer_State tokenizer(test_parse);
        auto t = parse_any_statement(tokenizer);
        assert(t && "This should at least pass parsing at minimum.");
        _debug_print_statement(t);
    }
#endif
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
        // Hmm, is this really a typechecking responsibility?
        // honestly, I think this is something I can check on usage for calling functions or expressions.
        // I believe it is best to do it then
        // However, this unused test case will be kept here so I can reference expected behavior.
        // NOTE:
        // I know this brings into question
        // what happens if a function taking an array reference/pointer decides to push or anything
        // in that case, I'll enforce a runtime check.
        // This language isn't meant to be "efficient".
        // assert(crank_type_match(lookup_type("int", {4}), lookup_type("int", {-1})) && "This is legal because I can promote the type");
        // assert(!crank_type_match(lookup_type("int", {-1}), lookup_type("int", {4})) && "This is illegal, because a fixed size array is inferior to a flexible one");
    }

    {
        printf("Function type checking.\n");

        assert(crank_type_match(
                   lookup_type("int", {}, {}, true),
                   lookup_type("int", {}, {}, true)
               ) && "Same function signature. Should match");

        // function returning array
        assert(crank_type_match(
                   lookup_type("int", {-1}, {}, true),
                   lookup_type("int", {-1}, {}, true)
               ) && "Same function signature. Should match");

        // Although like I said above, this is case that I would be okay with "type promoting".
        // However type promoting isn't the type matcher's problem. This situation
        assert(!crank_type_match(
                   lookup_type("int", {4}, {}, true),
                   lookup_type("int", {-1}, {}, true)
               ) && "Function signature differs. Should not match! I would type promote though.");
        assert(!crank_type_match(
                   lookup_type("int", {-1}, {}, true),
                   lookup_type("int", {4}, {}, true)
               ) && "Function signature differs. Should not match!");

        assert(!crank_type_match(
                   lookup_type("bool", {}, {}, true),
                   lookup_type("int", {}, {}, true)
               ) && "Function signature differs. Should not match!");

        assert(!crank_type_match(
                   lookup_type("int", {},
                               {
                                   make_uninitialized_object_decl(lookup_type("int"), "namesdonotmatter"),
                                   make_uninitialized_object_decl(lookup_type("float"), "namesdonotmatter1")
                               }, true),
                   lookup_type("int", {}, {}, true)
               ) && "Function signature differs. Should not match!");
        assert(crank_type_match(
                   lookup_type("int", {},
                               {
                                   make_uninitialized_object_decl(lookup_type("int"), "namesdonotmatter"),
                                   make_uninitialized_object_decl(lookup_type("float"), "namesdonotmatter1")
                               }, true),
                   lookup_type("int", {},
                               {
                                   make_uninitialized_object_decl(lookup_type("int"), "himynameis"),
                                   make_uninitialized_object_decl(lookup_type("float"), "thisisadifferentname")
                               }, true)
               ) && "Same function signature. Should match");
        assert(!crank_type_match(
                   lookup_type("int", {},
                               {
                                   make_uninitialized_object_decl(lookup_type("double"), "namesdonotmatter"),
                                   make_uninitialized_object_decl(lookup_type("bool"), "namesdonotmatter1")
                               }, true),
                   lookup_type("int", {},
                               {
                                   make_uninitialized_object_decl(lookup_type("int"), "himynameis"),
                                   make_uninitialized_object_decl(lookup_type("float"), "thisisadifferentname")
                               }, true)
               ) && "Differing function signatures. Same # of arguments.");
    }
    {
        printf("Pointer matching\n");
        assert(crank_type_match(lookup_type("int", {-1, -1, -1}, {}, false, 1), lookup_type("int", {-1, -1, -1}, {}, false, 1)) && "Same pointer types. should be okay");
        assert(!crank_type_match(lookup_type("int", {-1, -1, -1}, {}, false, 1), lookup_type("int", {-1, -1, -1}, {}, false, 2)) && "Different pointer types. Should not match.");
        assert(!crank_type_match(lookup_type("int", {}, {}, false, 1), lookup_type("float", {}, {}, false, 1)) && "Different pointer types. Should not match.");
        assert(crank_type_match(lookup_type("int", {}, {}, false, 1), lookup_type("int", {}, {}, false, 1)) && "Same pointer types. should be okay");
    }
}

void type_inference_test() {
    //?
}

void run_all_tests() {
    typecheck_matching();
    type_inference_test();
    parse_statements();
    parse_object_literal();
    parse_declaration();
}
