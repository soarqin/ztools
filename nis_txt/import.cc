#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char* argv[]) {
    FILE* f = fopen(argv[1], "r+b");
    if (f == nullptr) return 0;
    FILE* f2 = fopen(argv[2], "rt");
    if (f2 == nullptr) return 0;
    bool padding = (argc > 3) && strcmp(argv[3], "1") == 0;
    char line[4096];
    if (fgets(line, 4096, f2) == nullptr) return -1;
    uint32_t off_fix = (uint32_t)strtoul(line, nullptr, 10);
    while(fgets(line, 4096, f2) != nullptr) {
        int l = (int)strlen(line) - 1;
        while(l >= 0 && (line[l] == '\r' || line[l] == '\n')) --l;
        line[l + 1] = 0;
        char* n = strtok(line, ",");
        if(n == nullptr) continue;
        uint32_t offset = (uint32_t)strtoul(n, nullptr, 16);
        n = strtok(nullptr, ",");
        if(n == nullptr) continue;
        uint32_t count = (uint32_t)strtoul(n, nullptr, 10);
        n = strtok(nullptr, ",");
        if(n == nullptr) continue;
        uint32_t sl = (uint32_t)strlen(n);
        if (sl > count) {
            printf("WARN: Long string: %08X %u %u\n", offset, count, sl);
        } else {
            fseek(f, offset + off_fix, SEEK_SET);
            if (padding) {
                fwrite(n, 1, sl, f);
                uint32_t padlen = count - sl;
                while(padlen > 0) {
                    if (padlen >= 3) {
                        fwrite("\xE3\x80\x80", 1, 3, f);
                        padlen -= 3;
                    } else {
                        fwrite(" ", 1, 1, f);
                        --padlen;
                    }
                }
            } else
                fwrite(n, 1, sl + 1, f);
        }
        // printf("%08X %u %s\n", offset, count, n);
    }
    fclose(f);
    fclose(f2);
    return 0;
}
