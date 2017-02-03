#pragma once

class imy {
public:
    ~imy();
    bool extract(const char* filename, const char* outfile);
    bool build(const char* filename);
    void add(const char* file);

private:
    int child_count = 0;
    char** child_file = nullptr;
};
