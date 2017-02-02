#include "pch.hh"

#include "lz.hh"

#include "lzs.hh"

void printhelp() {
    fprintf(stderr, "Usage:\n"
        "  Compress:   lzs c <src file> <dst file>\n"
        "  Decompress: lzs d <src file> <dst file>\n");
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printhelp();
        return 0;
    }
    int op = 0;
    if (strcmp(argv[1], "c") == 0)
        op = 1;
    else if (strcmp(argv[1], "d") == 0)
        op = 0;
    else {
        printhelp();
        return 0;
    }
    lzs::reg();
    FILE* ifs = fopen(argv[2], "rb");
    if (ifs == nullptr) {
        fprintf(stderr, "Unable to open input file!\n");
        return -1;
    }
    lz_base* b;
    switch (op) {
    case 0:
        b = g_lz.detect(ifs);
        if (b == nullptr) {
            fclose(ifs);
            fprintf(stderr, "Unable to detect file type!\n");
            return -1;
        }
        break;
    case 1:
        b = g_lz.get("lzs");
        break;
    }
    FILE* ofs = fopen(argv[3], "wb");
    if (ofs == nullptr) {
        fclose(ifs);
        fprintf(stderr, "Unable to write into output file!\n");
        return -1;
    }
    op == 1 ? b->compress(ifs, ofs) : b->decompress(ifs, ofs);
    fclose(ifs);
    fclose(ofs);
    return 0;
}
