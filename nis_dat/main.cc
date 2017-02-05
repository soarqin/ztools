#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int op = -1;
    if (strcmp(argv[1], "e") == 0)
        op = 0;
    else if (strcmp(argv[1], "p") == 0)
        op = 1;
    switch(op) {
        case 0: {
            FILE* f = fopen(argv[2], "rb");
            if (f == nullptr) return -1;
            char path[256];
            sprintf(path, "%s/_list.txt", argv[3]);
            FILE* flist = fopen(path, "w");
            uint32_t count;
            fread(&count, 4, 1, f);
            uint32_t startoff = 16 + 32 * count;
            uint32_t laststart = 0;
            fprintf(flist, "%u\n", count);
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
                sprintf(path, "%s/%s", argv[3], fn);
                FILE* fout = fopen(path, "wb");
                if (fout == nullptr) {
                    delete[] data; continue;
                }
                fwrite(data, 1, endoff - laststart, fout);
                delete[] data;
                fclose(fout);
                laststart = endoff;
                fprintf(flist, "%s\n", fn);
            }
            fclose(f);
            fclose(flist);
            break;
        }
        case 1: {
            char path[256];
            sprintf(path, "%s/_list.txt", argv[3]);
            FILE* flist = fopen(path, "rt");
            char fn[256];
            memset(fn, 0, 256);
            if (fgets(fn, 256, flist) == nullptr) {
                fclose(flist);
                return -1;
            }
            uint32_t count = (uint32_t)strtoul(fn, nullptr, 10);
            FILE* f = fopen(argv[2], "wb");
            fwrite(&count, 4, 1, f);
            fseek(f, 16 + 32 * count, SEEK_SET);
            uint32_t totalsize = 0;
            uint32_t index = 0;
            while (fgets(fn, 256, flist) != nullptr) {
                int r = strlen(fn) - 1;
                while(r >= 0 && (fn[r] == '\r' || fn[r] == '\n')) --r;
                fn[r + 1] = 0;
                fprintf(stdout, "%s\n", fn);
                sprintf(path, "%s/%s", argv[3], fn);
                FILE* fd = fopen(path, "rb");
                fseek(fd, 0, SEEK_END);
                uint32_t sz = ftell(fd);
                fseek(fd, 0, SEEK_SET);
                uint8_t* data = new uint8_t[sz];
                fread(data, 1, sz, fd);
                fclose(fd);
                totalsize += sz;
                fwrite(data, 1, sz, f);
                fseek(f, 16 + 32 * index, SEEK_SET);
                fwrite(&totalsize, 4, 1, f);
                fn[27] = 0;
                fwrite(fn, 1, 28, f);
                ++index;
                delete[] data;
                memset(fn, 0, 256);
                fseek(f, 0, SEEK_END);
            }
            fclose(flist);
            fclose(f);
            break;
        }
        default: return -1;
    }
    return 0;
}
