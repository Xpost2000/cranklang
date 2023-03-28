#ifndef ERROR_H
#define ERROR_H

#include <cassert>

template <typename T>
struct Error {
    static Error fail(char* message) {
        Error result = {}; 
        result.good = false;
        result.message = message;
        return result;
    }

    static Error okay(T value) {
        Error result = {}; 
        result.good = true;
        result.value = value;
        result.message = "no errors.";
        return result;
    }

    operator T&() {
        assert(good);
        return value;
    }

    T value;
    bool good;
    char* message;
};

#endif
