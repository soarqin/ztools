#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

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
            fseek(f, 12, SEEK_SET);
            fread(&count, 4, 1, f);
            fprintf(flist, "%u\n", count);
            for (uint32_t i = 0; i < count; ++i) {
                fseek(f, 16 + 44 * i, SEEK_SET);
                char name[32];
                uint32_t off;
                uint32_t size;
                uint32_t tag;
                fread(name, 1, 32, f);
                fread(&off, 4, 1, f);
                fread(&size, 4, 1, f);
                fread(&tag, 4, 1, f);
                fseek(f, off, SEEK_SET);
                sprintf(path, "%s/%s", argv[3], name);
                FILE* fout = fopen(path, "wb");
                if (fout == nullptr) continue;
                uint8_t* data = new uint8_t[size];
                fread(data, 1, size, f);
                fwrite(data, 1, size, fout);
                delete[] data;
                fclose(fout);
                fprintf(flist, "%s 0x%08X\n", name, tag);
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
            fwrite("\x4E\x49\x53\x50\x41\x43\x4B\x00\x00\x00\x00\x00", 1, 12, f);
            fwrite(&count, 4, 1, f);
            uint32_t offset = ((16 + 44 * count - 1) & ~0x7FF) + 0x800;
            uint32_t index = 0;
            memset(fn, 0, 256);
            while (fgets(fn, 256, flist) != nullptr) {
                int r = strlen(fn) - 1;
                while(r >= 0 && (fn[r] == '\r' || fn[r] == '\n')) --r;
                fn[r + 1] = 0;
                char* name = strtok(fn, " ");
                char* v = strtok(nullptr, " ");
                uint32_t tag = strtoul(v, nullptr, 16);
                fprintf(stdout, "%s\n", fn);
                sprintf(path, "%s/%s", argv[3], fn);
                FILE* fd = fopen(path, "rb");
                fseek(fd, 0, SEEK_END);
                uint32_t sz = ftell(fd);
                fseek(fd, 0, SEEK_SET);
                uint8_t* data = new uint8_t[sz];
                fread(data, 1, sz, fd);
                fclose(fd);
                fseek(f, offset, SEEK_SET);
                fwrite(data, 1, sz, f);
                uint32_t endoff = (((uint32_t)ftell(f) - 1) & ~0x7FF) + 0x800;
                fseek(f, 16 + 44 * index, SEEK_SET);
                char filename[32];
                memset(filename, 0, 32);
                strcpy(filename, name);
                fwrite(filename, 1, 32, f);
                fwrite(&offset, 4, 1, f);
                fwrite(&sz, 4, 1, f);
                fwrite(&tag, 4, 1, f);
                ++index;
                delete[] data;
                memset(fn, 0, 256);
                offset = endoff;
            }
            fclose(flist);
            // ftruncate(fileno(f), offset);
            fclose(f);
            break;
        }
        default: return -1;
    }
    return 0;
}
