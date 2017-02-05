#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include "utf.hh"

#include "jp_old.inl"
#include "chs_old.inl"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct fnt_header {
    char name[0x28];
    uint32_t size;
    uint32_t offset;
};

static std::map<uint32_t, uint32_t> char_index;
static std::vector<uint32_t> chars;
static uint32_t fontw = 16, fonth = 16;

inline static uint32_t bswap(uint32_t n) {
    return ((n >> 24) & 0xFFu) | ((n >> 8) & 0xFF00u) | ((n << 8) & 0xFF0000u) | ((n << 24) & 0xFF000000u);
}

inline static void load_ffm(const uint8_t* data, size_t sz) {
    uint32_t count = *(uint16_t*)data;
    fontw = data[3]; fonth = data[4];
    chars.resize(count);
    memcpy(&chars[0], data + 0x10, count * 4);
    for(uint32_t i = 0; i < count; ++i) {
        chars[i] = bswap(chars[i]);
        auto c = chars[i];
        char_index[c] = i;
    }
}

inline static uint32_t find_char(uint32_t ch) {
    auto ite = char_index.find(ch);
    if (ite == char_index.end()) return 0;
    return ite->second;
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
    FILE* f = fopen(argv[1], "r+b");
    if (f == nullptr) return -1;
    fseek(f, 8, SEEK_SET);
    uint32_t count;
    fread(&count, 4, 1, f);
    fnt_header* header = new fnt_header[count];
    fseek(f, 0x10, SEEK_SET);
    fread(header, sizeof(fnt_header), count, f);

    for(uint32_t i = 0; i < count; ++i) {
        if(strcasecmp(header[i].name, "font.ffm") == 0) {
            uint8_t* data = new uint8_t[header[i].size];
            fseek(f, header[i].offset, SEEK_SET);
            fread(data, 1, header[i].size, f);
            load_ffm(data, header[i].size);
            delete[] data;
            break;
        }
    }

    stbtt_fontinfo font;
    size_t sz = 0;
    uint8_t* ttf_buffer = read_file("C:/Windows/Fonts/msyh.ttc", sz);

    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    float hh = stbtt_ScaleForPixelHeight(&font, fonth);
    // printf("%d %d %f\n", (int)ceil(-ascent * hh), (int)ceil(-descent * hh), hh);
    int tt = (int)ceil(-ascent * hh);

    uint32_t pitch = (fontw + 1) / 2;
    uint32_t texw = 512u / pitch;
    uint32_t texh = 1024u / fonth;
    uint32_t texc = texw * texh;
    for (uint32_t index = 0; index < (uint32_t)chars.size(); ++index) {
        uint32_t c = chars[index];
        if (c < 0x10000 || (c & 0xFF) < 0xE4) continue;
        uint32_t uc = utf8_to_utf16le(c);
        if (jp_old.find(uc) == jp_old.end()) {
            bool found = false;
            uint32_t u8c = 0;
            while(!chs_old.empty()) {
                uc = *chs_old.begin();
                chs_old.erase(chs_old.begin());
                u8c = utf16le_to_utf8(uc);
                if (find_char(u8c) == 0) {
                    found = true;
                    break;
                }
            }
            if (found) {
                char_index.erase(c);
                c = chars[index] = u8c;
                char_index[u8c] = index;
            }
        }
        if (index == 0) continue;
        uint32_t texindex = index / texc;
        uint32_t texx = (index % texc) % texw;
        uint32_t texy = (index % texc) / texw;
        // printf("Updating %X: %u %u %u\n", c, texindex+1, texx, texy);
        char fn[256];
        sprintf(fn, "font%02u.txp", texindex + 1);
        for(uint32_t i = 0; i < count; ++i) {
            if(strcasecmp(header[i].name, fn) == 0) {
                uint8_t *bitmap;
                int w, h, l, t;
                bitmap = stbtt_GetCodepointBitmap(&font, 0, hh, uc, &w, &h, &l, &t);
                if (bitmap == nullptr) break;
                uint8_t* fdata = new uint8_t[pitch * fonth];
                memset(fdata, 0, pitch * fonth);
                // printf("%p %d %d %d %d\n", bitmap, w, h, l, t);
                if (t + h - tt > fonth) {
                    printf("A: %X %u\n", c, t + h - tt);
                }
                if (l + w > fontw) {
                    printf("B: %X %u\n", c, l + w);
                }
                for (int j=tt; j < t+h; ++j) {
                    int y = j - tt;
                    if (j < t) continue;
                    for (int i=0; i < l+w; ++i) {
                        if (i < l) continue;
                        if (i % 2) {
                            fdata[pitch * y + i / 2] |= bitmap[(j-t)*w+i-l] & 0xF0;
                        } else {
                            fdata[pitch * y + i / 2] |= bitmap[(j-t)*w+i-l] >> 4;
                        }
                    }
                }
                stbtt_FreeBitmap(bitmap, 0);
                uint8_t* ptr = fdata;
                for (uint32_t j = 0; j < fonth; ++j) {
                    fseek(f, header[i].offset + 0x50 + 512 * (texy * fonth + j) + texx * pitch, SEEK_SET);
                    fwrite(ptr, 1, pitch, f);
                    ptr += pitch;
                }
                header[i].offset;
                delete[] fdata;
                break;
            }
        }
    }
    delete[] ttf_buffer;

    for(uint32_t i = 0; i < count; ++i) {
        if(strcasecmp(header[i].name, "font.ffm") == 0) {
            for (auto& c: chars) {
                c = bswap(c);
            }
            fseek(f, header[i].offset + 0x10, SEEK_SET);
            fwrite(&chars[0], 4, chars.size(), f);
            break;
        }
    }

    delete[] header;
    fclose(f);

    return 0;
}
