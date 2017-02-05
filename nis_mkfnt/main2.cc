#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include "utf.hh"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct fnt_header {
    char name[0x28];
    uint32_t size;
    uint32_t offset;
};

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

inline static uint32_t bswap(uint32_t n) {
    return ((n >> 24) & 0xFFu) | ((n >> 8) & 0xFF00u) | ((n << 8) & 0xFF0000u) | ((n << 24) & 0xFF000000u);
}

#include "chs_old.inl"
#include "jp_old.inl"
int main(int argc, char* argv[]) {
    std::set<uint32_t> nnn;
    for (auto& c: chs_old) nnn.insert(c);

    std::map<uint32_t, std::vector<uint8_t>> fontdata;
    std::vector<uint32_t> fonts;
    FILE* f = fopen(argv[1], "rb");
    if (f == nullptr) return -1;
    fseek(f, 8, SEEK_SET);
    uint32_t count;
    fread(&count, 4, 1, f);
    fnt_header* header = new fnt_header[count];
    fseek(f, 0x10, SEEK_SET);
    fread(header, sizeof(fnt_header), count, f);

    uint32_t fontw = 16, fonth = 16;
    uint32_t pitch, texw, texh, texc;

    for(uint32_t i = 0; i < count; ++i) {
        if(strcasecmp(header[i].name, "font.ffm") == 0) {
            uint8_t* data = new uint8_t[header[i].size];
            fseek(f, header[i].offset, SEEK_SET);
            fread(data, 1, header[i].size, f);

            uint32_t fcount = *(uint16_t*)data;
            fontw = data[3]; fonth = data[4];
            pitch = (fontw + 1) / 2;
            texw = 512u / pitch;
            texh = 1024u / fonth;
            texc = texw * texh;

            uint32_t* chars = (uint32_t*)(data + 0x10);
            for(uint32_t i = 0; i < fcount; ++i) {
                uint32_t c = bswap(chars[i]);
                if ((c & 0xFF) >= 0xE4 && ((c & 0xFF) < 0xEF || ((c & 0xFF) == 0xEF && ((c >> 8) & 0xFF) <= 0xA8))) {
                    nnn.insert(utf8_to_utf16le(c));
                    continue;
                }
                uint32_t uc = utf8_to_utf16le(c);
                fonts.push_back(uc);
                auto& v = fontdata[uc];
                v.resize(pitch * fonth);

                uint32_t texindex = i / texc;
                uint32_t texx = (i % texc) % texw;
                uint32_t texy = (i % texc) / texw;
                // printf("Updating %X: %u %u %u\n", c, texindex+1, texx, texy);
                char fn[256];
                sprintf(fn, "font%02u.txp", texindex + 1);
                for(uint32_t j = 0; j < count; ++j) {
                    if(strcasecmp(header[j].name, fn) == 0) {
                        fseek(f, header[j].offset + 0x08, SEEK_SET);
                        uint16_t palette_cnt;
                        fread(&palette_cnt, 2, 1, f);
                        if (palette_cnt == 0x10) {
                            for (uint32_t z = 0; z < fonth; ++z) {
                                fseek(f, header[j].offset + 0x50 + 512 * (texy * fonth + z) + texx * pitch, SEEK_SET);
                                fread(&v[pitch * z], 1, pitch, f);
                            }
                        } else if (palette_cnt == 0x100) {
                            for (uint32_t z = 0; z < fonth; ++z) {
                                fseek(f, header[j].offset + 0x410 + 1024 * (texy * fonth + z) + texx * fonth, SEEK_SET);
                                for (uint32_t y = 0; y < fontw; ++y) {
                                    uint8_t pixel;
                                    fread(&pixel, 1, 1, f);
                                    pixel -= 0xEF;
                                    if (pixel > 5) --pixel;
                                    if (y % 2)
                                        v[pitch * z + y / 2] |= pixel << 4;
                                    else
                                        v[pitch * z + y / 2] |= pixel;
                                }
                            }
                        }
                    }
                }
            }
            delete[] data;
            break;
        }
    }
    delete[] header;
    fclose(f);
    uint32_t allowed_size = 13 * texc - fonts.size();
    {
        auto ite = nnn.end();
        while(nnn.size() > allowed_size) {
            --ite;
            if (jp_old.find(*ite) == jp_old.end() && chs_old.find(*ite) == chs_old.end()) {
                ite = nnn.erase(ite);
            }
        }
    }

    stbtt_fontinfo font;
    size_t sz = 0;
    uint8_t* ttf_buffer = read_file("C:/Windows/Fonts/msyh.ttc", sz);

    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    float hh = stbtt_ScaleForPixelHeight(&font, fonth + 8);
    int tt = (int)ceil(-ascent * hh);

    for (auto& nn: nnn) {
        uint32_t index = (uint32_t)fonts.size();
        fonts.push_back(nn);
        uint8_t *bitmap;
        int w, h, l, t;
        bitmap = stbtt_GetCodepointBitmap(&font, 0, hh, nn, &w, &h, &l, &t);
        if (bitmap == nullptr) continue;
        auto& v = fontdata[nn];
        if(h > fonth) h = fonth;
        if(w > fontw) w = fontw;
        v.resize(pitch * fonth);
        if (t + h - tt > fonth) {
            t = fonth + tt - h;
        }
        if (l + w > fontw) {
            l = fontw - w;
        }
        for (int j=tt; j < t+h; ++j) {
            int y = j - tt;
            if (j < t) continue;
            for (int i=0; i < l+w; ++i) {
                if (i < l) continue;
                if (i % 2) {
                    v[pitch * y + i / 2] |= bitmap[(j-t)*w+i-l] & 0xF0;
                } else {
                    v[pitch * y + i / 2] |= bitmap[(j-t)*w+i-l] >> 4;
                }
            }
        }
        stbtt_FreeBitmap(bitmap, 0);
    }

    if (fonts.empty()) return 0;
    uint32_t fsize = (uint32_t)fonts.size();
    uint32_t fcount = (fsize - 1) / texc + 1 + 1;
    if (fcount < 14) fcount = 14;

    f = fopen("FONT.DAT", "w+b");
    fwrite("DSARC FL", 1, 8, f);
    fwrite(&fcount, 4, 1, f);
    uint32_t startoff = ((0x10 + 0x30 * fcount - 1) & ~0x1FF) + 0x200;
    fnt_header* hdr = new fnt_header[fcount];
    memset(hdr, 0, fcount * sizeof(fnt_header));
    strcpy(hdr[0].name, "FONT.FFM");
    hdr[0].size = ((fsize * 4 + 0x10 - 1) & ~0x1FF) + 0x200;
    hdr[0].offset = startoff;
    startoff = startoff + hdr[0].size;
    fseek(f, hdr[0].offset, SEEK_SET);
    fwrite(&fsize, 3, 1, f);
    fwrite(&fontw, 1, 1, f);
    fwrite(&fonth, 1, 1, f);
    fseek(f, hdr[0].offset + 0x10, SEEK_SET);
    std::sort(fonts.begin(), fonts.end());
    for(auto cc: fonts) {
        auto c = bswap(utf16le_to_utf8(cc));
        fwrite(&c, 4, 1, f);
    }

    uint8_t* data = new uint8_t[0x80200];
    for(uint32_t i = 1; i < fcount; ++i) {
        memset(data, 0, 0x80200);
        const uint32_t color_table[16] = {0, 0x10FFFFFF, 0x20FFFFFF, 0x30FFFFFF,
        0x40FFFFFF, 0x5AFFFFFF, 0x70FFFFFF, 0x80FFFFFF,
        0x90FFFFFF, 0xA0FFFFFF, 0xB0FFFFFF, 0xC0FFFFFF,
        0xD0FFFFFF, 0xE0FFFFFF, 0xF0FFFFFF, 0xFFFFFFFF};
        const uint8_t txp_header[16] = {0x00, 0x04, 0x00, 0x04, 0x10, 0x00, 0x0A, 0x0A, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
        memcpy(data, txp_header, 0x10);
        memcpy(data + 0x10, color_table, 0x40);
        uint32_t index = texc * (i - 1);
        uint32_t iend = index + texc;
        if (iend > fsize) iend = fsize;
        for(; index < iend; ++index) {
            auto& v = fontdata[fonts[index]];
            if (v.empty()) continue;
            uint32_t texx = (index % texc) % texw;
            uint32_t texy = (index % texc) / texw;
            for (uint32_t z = 0; z < fonth; ++z) {
                memcpy(data + 0x50 + 512 * (texy * fonth + z) + texx * pitch, &v[pitch * z], pitch);
            }
        }
        sprintf(hdr[i].name, "FONT%02u.TXP", i);
        hdr[i].offset = startoff;
        hdr[i].size = 0x80050;
        startoff += 0x80200;
        fseek(f, hdr[i].offset, SEEK_SET);
        fwrite(data, 1, 0x80200, f);
    }
    delete[] data;
    fseek(f, 0x10, SEEK_SET);
    fwrite(hdr, sizeof(fnt_header), fcount, f);
    fclose(f);
/*
    stbtt_fontinfo font;
    size_t sz = 0;
    uint8_t* ttf_buffer = read_file("C:/Windows/Fonts/msyh.ttc", sz);

    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    float hh = stbtt_ScaleForPixelHeight(&font, fonth);
    // printf("%d %d %f\n", (int)ceil(-ascent * hh), (int)ceil(-descent * hh), hh);
    int tt = (int)ceil(-ascent * hh);

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
*/
    return 0;
}
