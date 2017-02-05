#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <map>

inline void load_codepage(const char* fn, std::map<uint32_t, uint32_t>& cp) {
    FILE* f = fopen(fn, "rt");
    char line[256];
    while(fgets(line, 256, f) != nullptr) {
        char* t = strtok(line, "=");
        if (t == nullptr) continue;
        uint32_t key = (uint32_t)strtoul(t, nullptr, 16);
        t = strtok(nullptr, "=");
        if (t == nullptr) continue;
        uint32_t& val = cp[key];
        val = 0;
        while((uint8_t)*t >= 0x20) {
            val = (val << 8) | (uint8_t)*t;
            ++t;
        }
    }
    fclose(f);
}

inline void convert(const char* o, char* r, const std::map<uint32_t, uint32_t>& cp) {
    while(*o != 0) {
        if((uint32_t)*o < 0x80) {
            *r++ = *o++;
        } else {
            uint32_t key = ((uint16_t)(uint8_t)o[0] << 8) + (uint8_t)o[1];
            auto ite = cp.find(key);
            if (ite == cp.end()) *r++ = '?';
            else {
                uint32_t n = ite->second;
                if (n < 0x100) *r++ = (char)n;
                else if (n < 0x10000) {
                    *r++ = (char)(n >> 8);
                    *r++ = (char)(n & 0xFF);
                } else {
                    *r++ = (char)((n >> 16) & 0xFF);
                    *r++ = (char)((n >> 8) & 0xFF);
                    *r++ = (char)(n & 0xFF);
                }
            }
            o += 2;
        }
    }
    *r = 0;
}

int main(int argc, char* argv[]) {
    bool use_cp = false;
    std::map<uint32_t, uint32_t> cp;
    if (strcasecmp(argv[2], "u") != 0) {
        use_cp = true;
        load_codepage(argv[2], cp);
    }
    switch(argv[1][0]) {
        case 'c': {
            FILE* f = fopen("char.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("char.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x160];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0xF6 : 0x160, f);
                if (use_cp) {
                    char realname[64];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x16, realname);
                    convert((char*)data + 0x17, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x17, 0x16, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x33, (char*)data);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x34, 0x33, (char*)data + 0x34);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'h': {
            FILE* f = fopen("charhelp.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("charhelp.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x120];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x98 : 0x120, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x31, realname);
                    convert((char*)data + 0x32, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x32, 0x31, realname);
                    convert((char*)data + 0x64, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x64, 0x31, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x5B, (char*)data);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x5C, 0x5B, (char*)data + 0x5C);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0xB8, 0x5B, (char*)data + 0xB8);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'd': {
            FILE* f = fopen("dungeon.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("dungeon.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x80];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x50 : 0x80, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x4F, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x7F, (char*)data);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'b': {
            FILE* f = fopen("habit.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("habit.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x80];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x40 : 0x80, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x3F, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x7F, (char*)data);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'm': {
            FILE* f = fopen("magic.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("magic.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x100];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x70 : 0x100, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data + 0x0A, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x0A, 0x16, realname);
                    convert((char*)data + 0x21, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x21, 0x37, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x0A, 0x33, (char*)data + 0x0A);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x3E, 0x7F, (char*)data + 0x3E);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'i': {
            FILE* f = fopen("mitem.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("mitem.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x100];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x78 : 0x100, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x14, realname);
                    convert((char*)data + 0x15, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x15, 0x37, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x43, (char*)data);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x44, 0x7F, (char*)data + 0x44);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 's': {
            FILE* f = fopen("music.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("music.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0xC0];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x68 : 0xC0, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data + 0x10, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x10, 0x17, realname);
                    convert((char*)data + 0x28, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x28, 0x3F, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x10, 0x2B, (char*)data + 0x10);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x3C, 0x77, (char*)data + 0x3C);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
    }
    return 0;
}
