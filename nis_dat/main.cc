#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
    FILE* f = fopen(argv[1], "rb");
    if (f == nullptr) return -1;
    uint32_t count;
    fread(&count, 4, 1, f);
    uint32_t startoff = 16 + 32 * count;
    uint32_t laststart = 0;
    for (uint32_t i = 0; i < count; ++i) {
        fseek(f, 16 + 32 * i, SEEK_SET);
        uint32_t endoff;
        fread(&endoff, 4, 1, f);
        char fn[28];
        fread(fn, 1, 28, f);
        fn[27] = 0;
        fseek(f, startoff + laststart, SEEK_SET);
        uint8_t* data = new uint8_t[endoff - laststart];
        fread(data, 1, endoff - laststart, f);
        char path[256];
        sprintf(path, "%s/%s", argv[2], fn);
        FILE* fout = fopen(path, "wb");
        if (fout == nullptr) {
            delete[] data; continue;
        }
        fwrite(data, 1, endoff - laststart, fout);
        delete[] data;
        fclose(fout);
        laststart = endoff;
    }
    fclose(f);
    return 0;
}
