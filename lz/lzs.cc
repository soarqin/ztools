#include "pch.hh"

#include "lzs.hh"

#include "lz.hh"

#include <cctype>

void lzs::reg() {
    g_lz.reg("lzs", new lzs);
}

struct lzs_index {
    inline void add(uint32_t off) {
        if (length < 256) {
            offset[(startoff + length++) & 0xFF] = off;
        } else {
            offset[startoff++] = off;
            startoff = startoff & 0xFF;
        }
    }
    inline void purge(uint32_t off) {
        while(length > 0 && offset[startoff] < off) {
            startoff = (startoff + 1) & 0xFF;
            --length;
        }
    }
    uint32_t offset[256];
    uint16_t startoff = 0;
    uint16_t length = 0;
};

bool lzs::detect(FILE* ifs) {
    uint32_t csize;
    char fe[4];
    fseek(ifs, 0, SEEK_SET);
    fread(fe, 4, 1, ifs);
    if (std::isalnum(fe[0]) && (fe[1] == 0 || std::isalnum(fe[1])) && (fe[2] == 0 || std::isalnum(fe[2]))) {
        fseek(ifs, 8, SEEK_SET);
        fread(&csize, 4, 1, ifs);
        fseek(ifs, 0, SEEK_END);
        return (uint32_t)ftell(ifs) == csize + 4;
    }
    return false;
}

void lzs::decompress(FILE* ifs, FILE* ofs) {
    uint32_t fe, usize, csize, cflag;
    uint8_t compflag;
    fseek(ifs, 0, SEEK_SET);
    fread(&fe, 4, 1, ifs);
    fread(&usize, 4, 1, ifs);
    fread(&csize, 4, 1, ifs);
    fread(&cflag, 4, 1, ifs);
    compflag = static_cast<uint8_t>(cflag);
    uint8_t* ss = new uint8_t[usize];
    uint8_t* ptr = ss;
    uint8_t* ptrend = ptr + usize;
    while (ptr < ptrend) {
        uint8_t b;
        fread(&b, 1, 1, ifs);
        if (b != compflag) {
            *ptr++ = b;
        } else {
            uint8_t distance;
            fread(&distance, 1, 1, ifs);
            if (distance == compflag) {
                *ptr++ = compflag;
            } else {
                if (distance > compflag) --distance;
                uint8_t length;
                fread(&length, 1, 1, ifs);
                memcpy(ptr, ptr - distance, length);
                ptr += length;
            }
        }
    }
    fwrite(ss, 1, usize, ofs);
    delete[] ss;
}

void lzs::compress(FILE* ifs, FILE* ofs) {
    fseek(ifs, 0, SEEK_END);
    uint32_t l = ftell(ifs);
    uint8_t* data = new uint8_t[l];
    fseek(ifs, 0, SEEK_SET);
    fread(&data[0], 1, l, ifs);
    uint32_t inoff = 0;
    uint8_t* ss = new uint8_t[l * 16 / 15];
    uint8_t* ptr = ss;
    lzs_index rindex[256];
    while(inoff < l) {
        uint8_t c = data[inoff];
        if ((inoff & 0xFFFF) == 0)
            fprintf(stdout, "\r%u/%u", inoff, l);
        lzs_index& v = rindex[c];
        uint8_t maxlen = 0;
        uint8_t maxoff = 0;
        if(v.length > 0) {
            if (inoff > 254) v.purge(inoff - 254);
            uint16_t voff = (v.startoff + v.length - 1) & 0xFF;
            for (uint16_t i = 0; i < v.length; ++i) {
                uint32_t off = v.offset[voff];
                if (off + 3 < inoff) {
                    uint8_t j = 1;
                    uint8_t mx = inoff - off;
                    while(j < mx && data[off + j] == data[inoff + j]) ++j;
                    if (j > maxlen) {
                        maxlen = j;
                        maxoff = mx;
                    }
                }
                if (voff > 0) --voff; else voff = 0xFF;
            }
        }
        if (maxlen >= 4) {
            *ptr++ = comp_flag;
            *ptr++ = maxoff >= comp_flag ? (maxoff + 1) : maxoff;
            *ptr++ = maxlen;
            for (uint8_t i = 0; i < maxlen; ++i)
                rindex[data[inoff + i]].add(inoff + i);
            inoff += maxlen;
        } else {
            if (c == comp_flag)
                *ptr++ = comp_flag;
            *ptr++ = c;
            rindex[c].add(inoff);
            ++inoff;
        }
    }
    delete[] data;
    fprintf(stdout, "\r%u/%u\n", inoff, l);
    fwrite(&file_ext, 4, 1, ofs);
    fwrite(&l, 4, 1, ofs);
    uint32_t sz = ptr - ss + 0x0C;
    fwrite(&sz, 4, 1, ofs);
    uint32_t cflag = comp_flag;
    fwrite(&cflag, 4, 1, ofs);
    fwrite(ss, 1, ptr - ss, ofs);
    delete[] ss;
}

void lzs::set_compress_option(uint32_t index, uint32_t value) {
    switch(index) {
    case 0:
        file_ext = value;
        break;
    case 1:
        comp_flag = value;
        break;
    default:
        break;
    }
}
