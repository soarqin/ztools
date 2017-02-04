#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vector>

int main(int argc, char* argv[]) {
    FILE* f = fopen(argv[2], "rb");
    if (f == nullptr) return -1;
    std::vector<uint16_t> char_tbl;
    while(!feof(f)) {
        uint16_t ch;
        if(fread(&ch, 2, 1, f) > 0 && ch >= 0x20)
            char_tbl.push_back(ch);
    }
    fclose(f);
    f = fopen(argv[1], "rb");
    if (f == nullptr) return -1;
    uint16_t count;
    fread(&count, 2, 1, f);
    uint16_t* data = new uint16_t[count];
    fread(data, 2, count, f);
    fclose(f);
    uint16_t cc = 0x20;
    FILE* ftab = fopen(argv[3], "wb");
    for(uint16_t i = 0; i < count; ++i) {
        if(data[i] != 0) {
            wchar_t wbuf[256];
            uint16_t n = char_tbl[data[i]];
            if (n < 0x100) {
                swprintf(wbuf, 256, L"%02X=%lc\r\n", cc, char_tbl[data[i]]);
            } else {
                swprintf(wbuf, 256, L"%04X=%lc\r\n", cc, char_tbl[data[i]]);
            }
            fwrite(wbuf, 2, wcslen(wbuf), ftab);
        }
        ++cc;
        if (cc == 0x7F) cc = 0x8140;
        else if (cc > 0x100 && (cc & 0xFF) == 0) {
            cc += 0x40;
            if ((cc >> 8) == 0xA0) cc += 0x4000;
        }
    }
    delete[] data;
    fclose(ftab);
    return 0;
}
