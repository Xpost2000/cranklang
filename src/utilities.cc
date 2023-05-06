/* compile into crank.cc! */
struct File_Buffer {
    File_Buffer(const char* what);
    ~File_Buffer();

    operator const char*() {
        return data;
    }

    char* data = nullptr;
    size_t length = 0;
};

File_Buffer::File_Buffer(const char* where) {
    FILE* f = fopen(where, "rb+");
    if (!f) {
        // bad.
    } else {
        fseek(f, 0, SEEK_END);
        size_t length = ftell(f);
        fseek(f, 0, SEEK_SET);

        this->length       = length;
        this->data         = new char[length+1];
        this->data[length] = 0;

        fread((void*)this->data, length, 1, f);
    }

    fclose(f);
}

File_Buffer::~File_Buffer() {
    if (data) {
        delete[] data;
    }
}
