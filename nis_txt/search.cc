#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <map>
#include <set>

#include "utf.hh"

static std::map<uint32_t, uint16_t> cp;
static std::set<uint32_t> uset;

inline void load_codepage(const char* fn) {
    FILE* f = fopen(fn, "rt");
    char line[256];
    while(fgets(line, 256, f) != nullptr) {
        char* t = strtok(line, "=");
        if (t == nullptr) continue;
        uint32_t key = (uint32_t)strtoul(t, nullptr, 16);
        t = strtok(nullptr, "=");
        if (t == nullptr) continue;
        uint8_t s[7];
        int i = 0;
        while(i < 6 && (uint8_t)*t >= 0x20) {
            s[i] = (uint8_t)*t;
            ++t;
            ++i;
        }
        uint16_t o[7];
        int outlen = 6;
        int inlen = i;
        if(utf8_to_utf16le(o, &outlen, s, &inlen) == 0) {
            cp[key] = o[0];
            uset.insert(o[0]);
        }
    }
    fclose(f);
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

inline bool valid_str(const char* string, uint16_t* output) {
    const uint8_t* ptr = (const uint8_t*)string;
    uint16_t* optr = output;
    while(*ptr != 0) {
        uint8_t c = *ptr++;
        if(c < 0x80) {
            auto ite = cp.find(c);
            if(ite == cp.end()) return false;
            if(optr) *optr++ = c;
        } else {
            uint32_t r = ((uint32_t)c << 8) | (*ptr++);
            auto ite = cp.find(r);
            if(ite == cp.end()) return false;
            if(optr) *optr++ = ite->second;
        }
    }
    *optr = 0;
    return true;
}


int main(int argc, char* argv[]) {
    bool u8 = false;
    if (argc > 2) {
        load_codepage(argv[2]);
    } else u8 = true;
    size_t sz;
    uint8_t* data = read_file(argv[1], sz);
    if (data == nullptr) return -1;
    uint8_t* rdata = new uint8_t[sz];
    for (size_t i = 0; i < sz; ++i)
        rdata[i] = data[i] < 0x20 ? 0 : data[i];
    if (u8) {
        for (size_t i = 0; i < sz; ++i) {
            const char* s = (const char*)rdata + i;
            uint32_t cnt[4] = {0, 0, 0, 0};
            if(is_utf8(s, cnt)) {
                if (cnt[0] > 0) continue;
                uint32_t l = (uint32_t)strlen(s);
                if (l < 3 || data[i + l] != 0) {
                    continue;
                }
                size_t j = i;
                uint32_t ll = l;
                while (ll >= 3 && *(rdata + j) == 0xE3 && *(rdata + j + 1) == 0x80 && *(rdata + j + 2) == 0x80) {
                    ll -= 3; j += 3;
                }
                if (ll < 3) continue;
                fprintf(stdout, "0x%08X,%u,%s\n", i, l, s);
                i += l;
            }
        }
    } else {
        for (size_t i = 0; i < sz; ++i) {
            const char* s = (const char*)rdata + i;
            uint16_t out[32768];
            if(valid_str(s, out)) {
                uint32_t l = (uint32_t)strlen(s);
                if (l < 2 || data[i + l] != 0) {
                    continue;
                }
                int inlen = wcslen((wchar_t*)out);
                uint32_t cnt = 0;
                for(int j = 0; j < inlen; ++j) {
                    if(out[j] > 0x80) ++cnt;
                }
                if (cnt == 0) continue;
                uint8_t* uout = new uint8_t[inlen * 4 + 1];
                int outlen = inlen * 4 + 1;
                if (utf16le_to_utf8(uout, &outlen, out, &inlen) != 0) continue;
                if (outlen == 0) continue;
                uout[outlen] = 0;
                fprintf(stdout, "0x%08X,%u,%s\n", i, l, uout);
                i += l;
            }
        }
    }

    return 0;
}
