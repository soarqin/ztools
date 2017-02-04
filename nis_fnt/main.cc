#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iconv.h>

int main(int argc, char* argv[]) {
    FILE* f = fopen(argv[1], "rb");
    if (f == nullptr) return -1;
    iconv_t cd = iconv_open("GBK", "SJIS");
    uint16_t count;
    fread(&count, 2, 1, f);
    uint16_t* data = new uint16_t[count];
    fread(data, 2, count, f);
    fclose(f);
    uint16_t cc = 0x20;
    FILE* fout = fopen(argv[2], "wb");
    FILE* ftab = fopen(argv[3], "wt");
    for(uint16_t i = 0; i < count; ++i) {
        if(data[i] != 0) {
            if (cc < 0x100) {
                fwrite(&cc, 1, 1, fout);
                fprintf(ftab, "%02X=%c\n", cc, cc);
            } else {
                char din[16];
                char dout[16];
                din[0] = cc >> 8; din[1] = cc & 0xFF; din[2] = 0;
                fwrite(din, 1, 2, fout);
                size_t inl = 2, outl = 10;
                printf("%04X\n", cc);
                char* inptr = din;
                char* outptr = dout;
                iconv(cd, &inptr, &inl, &outptr, &outl);
                *outptr = 0;
                fprintf(ftab, "%02X=%s\n", cc, dout);
            }
        }
        ++cc;
        if (cc == 0x7F) cc = 0x8140;
        else if (cc > 0x100 && (cc & 0xFF) == 0) {
            cc += 0x40;
            if ((cc >> 8) == 0xA0) cc += 0x4000;
        }
    }
    delete[] data;
    iconv_close(cd);
    fclose(fout);
    fclose(ftab);
    return 0;
}
