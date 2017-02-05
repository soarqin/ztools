#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int op = -1;
    if(strcmp(argv[1], "e") == 0)
        op = 0;
    else if(strcmp(argv[1], "r") == 0)
        op = 1;
    switch(op) {
        case 0: {
            FILE* f = fopen(argv[2], "rb");
            if (f == nullptr) return 0;
            char header[16];
            fread(header, 1, 16, f);
            char fn[0x2C];
            uint32_t offset, size;
            while(!feof(f)) {
                if(fread(fn, 1, 0x2C, f) < 0x2C || fn[0] == 0) break;
                fread(&size, 4, 1, f);
                fread(&offset, 4, 1, f);
                if (strcasecmp(fn, argv[3]) == 0) {
                    fseek(f, offset, SEEK_SET);
                    uint8_t* data = new uint8_t[size];
                    fread(data, 1, size, f);
                    FILE* fout = fopen(argv[2], "wb");
                    fwrite(data, 1, size, fout);
                    fclose(fout);
                    delete[] data;
                    break;
                }
            }
            fclose(f);
            break;
        }
        case 1: {
            FILE* f = fopen(argv[2], "r+b");
            if (f == nullptr) return 0;
            char header[16];
            fread(header, 1, 16, f);
            char fn[0x2C];
            uint32_t offset, size;
            while(!feof(f)) {
                if(fread(fn, 1, 0x2C, f) < 0x2C || fn[0] == 0) break;
                fread(&size, 4, 1, f);
                fread(&offset, 4, 1, f);
                if (strcasecmp(fn, argv[3]) == 0) {
                    FILE* fin = fopen(argv[3], "rb");
                    if (fin == nullptr) return -1;
                    fseek(fin, 0, SEEK_END);
                    uint32_t insize = ftell(fin);
                    fseek(fin, 0, SEEK_SET);
                    uint8_t* data = new uint8_t[insize];
                    fread(data, 1, insize, fin);
                    fclose(fin);
                    fseek(f, -8, SEEK_CUR);
                    fwrite(&insize, 4, 1, f);
                    if(insize > size) {
                        uint32_t toff = ftell(f);
                        fseek(f, 0, SEEK_END);
                        uint32_t newoff = ftell(f);
                        fwrite(data, 1, insize, f);
                        fseek(f, toff, SEEK_SET);
                        fwrite(&newoff, 4, 1, f);
                    } else {
                        fseek(f, offset, SEEK_SET);
                        fwrite(data, 1, insize, f);
                    }
                    delete[] data;
                    fclose(fin);
                    break;
                }
            }
            fclose(f);
            break;
        }
        default:
            return -1;
    }
    return 0;
}
