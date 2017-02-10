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

inline static uint8_t* read_file(const char* fn, size_t& sz) {
    FILE* f = fopen(fn, "rb");
    if (f == nullptr) {
        return nullptr;
        sz = 0;
    }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz == 0) {
        fclose(f);
        return nullptr;
    }
    fseek(f, 0, SEEK_SET);
    uint8_t* data = new uint8_t[sz];
    fread(data, 1, sz, f);
    fclose(f);
    return data;
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
        case 'a': {
            if (use_cp) {
                size_t sz;
                uint8_t* data = read_file("InProgramTxtDB.dat", sz);
                FILE* fout = fopen("InProgramTxtDB.txt", "wt");
                uint32_t count = *(uint32_t*)data;
                uint32_t pos = 8;
                for(uint32_t i = 0; i < count; ++i) {
                    char* str = (char*)(data + pos);
                    uint32_t l = strlen(str);
                    char realname[1024];
                    convert(str, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", pos, l, realname);
                    pos += l + 2;
                }
                delete[] data;
                fclose(fout);
            } else {
                size_t sz;
                uint8_t* data = read_file("STRINGLISTDATABASE.dat", sz);
                FILE* fout = fopen("STRINGLISTDATABASE.txt", "wt");
                uint32_t count = *(uint32_t*)data;
                for(uint32_t i = 0; i < count; ++i) {
                    uint32_t offset = *(uint32_t*)(data + 8 + 8 * i);
                    char* str = (char*)(data + 4 + offset);
                    uint32_t l = strlen(str);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, l, str);
                }
                delete[] data;
                fclose(fout);
            }
            break;
        }
        case 'W': {
            FILE* f = fopen("wish.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("wish.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x100];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x5C : 0x100, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data + 0x0A, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x0A, 0x1E, realname);
                    convert((char*)data + 0x29, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x29, 0x2E, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x0A, 0x40, (char*)data + 0x0A);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x4E, 0x67, (char*)data + 0x4E);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'w': {
            FILE* f = fopen("wish2.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("wish2.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x140];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x8C : 0x140, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data + 0x28, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x28, 0x2E, realname);
                    convert((char*)data + 0x57, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x57, 0x2E, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x26, 0x67, (char*)data + 0x26);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x8E, 0x67, (char*)data + 0x8E);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'e': {
            const char* filename[2] = {"darksun1.dat", "darksun2.dat"};
            const char* outfile[2] = {"darksun1.txt", "darksun2.txt"};
            for(int i = 0; i < 2; ++i) {
                FILE* f = fopen(filename[i], "rb");
                if (f == nullptr) continue;
                FILE* fout = fopen(outfile[i], "wt");
                uint32_t count;
                fread(&count, 4, 1, f);
                if (use_cp)
                    fseek(f, 4, SEEK_CUR);
                uint8_t data[0x60];
                for(uint32_t i = 0; i < count; ++i) {
                    uint32_t offset = ftell(f);
                    fread(data, 1, use_cp ? 0x28 : 0x60, f);
                    if (use_cp) {
                        char realname[128];
                        convert((char*)data, realname, cp);
                        fprintf(fout, "0x%08X,%u,%s\n", offset, 0x16, realname);
                    } else {
                        fprintf(fout, "0x%08X,%u,%s\n", offset, 0x30, (char*)data);
                    }
                }
                fclose(f);
                fclose(fout);
            }
            break;
        }
        case 't': {
            FILE* f = fopen("THIEF.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("THIEF.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x40];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x1C : 0x40, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x17, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x2C, (char*)data);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'j': {
            FILE* f = fopen("judgment.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("judgment.txt", "wt");
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
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x1F, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x3F, (char*)data);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'n': {
            size_t sz;
            uint8_t* data = read_file("name.dat", sz);
            FILE* fout = fopen("name.txt", "wt");
            uint16_t* count = (uint16_t*)data;
            uint16_t* off[5];
            uint32_t start = 10;
            for (int i = 0; i < 5; ++i) {
                off[i] = (uint16_t*)(data + start);
                start += 2 * count[i];
            }
            for (int i = 0; i < 5; ++i) {
                char* str;
                uint32_t l;
                for (uint16_t j = 0; j < count[i]; ++j) {
                    uint32_t offset = start + off[i][j];
                    char* str = (char*)data + offset;
                    l = strlen(str);
                    if (use_cp) {
                        char realname[128];
                        convert(str, realname, cp);
                        fprintf(fout, "0x%08X,%u,%s\n", offset, l, realname);
                    } else {
                        fprintf(fout, "0x%08X,%u,%s\n", offset, l, str);
                    }
                }
            }
            delete[] data;
            fclose(fout);
            break;
        }
        case 'p': {
            FILE* f = fopen("pirate.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("pirate.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0xA0];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x7A : 0xA0, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x0F, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset, 0x1F, (char*)data);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'S': {
            FILE* f = fopen("senator.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("senator.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x60];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x40 : 0x60, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data + 1, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 1, 0x16, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 1, 0x1F, (char*)data + 1);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
        case 'z': {
            FILE* f = fopen("zukan.dat", "rb");
            if (f == nullptr) break;
            FILE* fout = fopen("zukan.txt", "wt");
            uint32_t count;
            fread(&count, 4, 1, f);
            if (use_cp)
                fseek(f, 4, SEEK_CUR);
            uint8_t data[0x340];
            for(uint32_t i = 0; i < count; ++i) {
                uint32_t offset = ftell(f);
                fread(data, 1, use_cp ? 0x184 : 0x340, f);
                if (use_cp) {
                    char realname[128];
                    convert((char*)data + 4, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 4, 0x14, realname);
                    convert((char*)data + 0x19, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x19, 0x2C, realname);
                    convert((char*)data + 0x46, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x46, 0x2C, realname);
                    convert((char*)data + 0x73, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x73, 0x2C, realname);
                    convert((char*)data + 0xA0, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0xA0, 0x2C, realname);
                    convert((char*)data + 0xCD, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0xCD, 0x2C, realname);
                    convert((char*)data + 0xFA, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0xFA, 0x2C, realname);
                    convert((char*)data + 0x127, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x127, 0x2C, realname);
                    convert((char*)data + 0x154, realname, cp);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x154, 0x2C, realname);
                } else {
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 4, 0x43, (char*)data + 4);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x48, 0x5B, (char*)data + 0x48);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0xA4, 0x5B, (char*)data + 0xA4);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x100, 0x5B, (char*)data + 0x100);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x15C, 0x5B, (char*)data + 0x15C);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x1B8, 0x5B, (char*)data + 0x1B8);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x214, 0x5B, (char*)data + 0x214);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x270, 0x5B, (char*)data + 0x270);
                    fprintf(fout, "0x%08X,%u,%s\n", offset + 0x2CC, 0x5B, (char*)data + 0x2CC);
                }
            }
            fclose(f);
            fclose(fout);
            break;
        }
    }
    return 0;
}
