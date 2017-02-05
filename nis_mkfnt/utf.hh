#pragma once

#include <string.h>

inline static int utf8_to_utf16le(uint16_t* out, int *outlen,
            const uint8_t* in, int *inlen)
{
    const uint8_t* processed = in;
    const uint8_t *const instart = in;
    uint16_t* outstart = out;
    uint16_t* outend;
    const uint8_t* inend= in+*inlen;
    uint32_t c, d;
    int trailing;
    uint8_t *tmp;
    uint16_t tmp1, tmp2;

    /* UTF16LE encoding has no BOM */
    if (in == NULL) {
        *outlen = 0;
        *inlen = 0;
        return(0);
    }
    outend = out + *outlen;
    while (in < inend) {
        d= *in++;
        if      (d < 0x80)  { c= d; trailing= 0; }
        else if (d < 0xC0) {
            /* trailing byte in leading position */
            *outlen = out - outstart;
            *inlen = processed - instart;
            return(-2);
        } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
        else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
        else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
        else {
            /* no chance for this in UTF-16 */
            *outlen = out - outstart;
            *inlen = processed - instart;
            return(-2);
        }

        if (inend - in < trailing) {
            break;
        } 

        for ( ; trailing; trailing--) {
            if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
                break;
            c <<= 6;
            c |= d & 0x3F;
        }

        /* assertion: c is a single UTF-4 value */
        if (c < 0x10000) {
            if (out >= outend)
                break;
            *out++ = c;
        } else if (c < 0x110000) {
            if (out+1 >= outend)
            break;
            c -= 0x10000;
            *out++ = 0xD800 | (c >> 10);
            *out++ = 0xDC00 | (c & 0x03FF);
        }
        else
            break;
        processed = in;
    }
    *outlen = out - outstart;
    *inlen = processed - instart;
    return(0);
}

inline static int utf16le_to_utf8(uint8_t* out, int *outlen,
            const uint16_t* in, int *inlen)
{
    uint8_t* outstart = out;
    const uint16_t* processed = in;
    uint8_t* outend = out + *outlen;
    const uint16_t* inend;
    uint32_t c, d;
    uint8_t *tmp;
    int bits;

    inend = in + *inlen;
    while ((in < inend) && (out - outstart + 5 < *outlen)) {
        c= *in++;
        if ((c & 0xFC00) == 0xD800) {    /* surrogates */
            if (in >= inend) {           /* (in > inend) shouldn't happens */
                break;
            }
            d = *in++;
            if ((d & 0xFC00) == 0xDC00) {
                c &= 0x03FF;
                c <<= 10;
                c |= d & 0x03FF;
                c += 0x10000;
            } else {
                *outlen = out - outstart;
                *inlen = processed - in;
                return(-2);
            }
        }

        /* assertion: c is a single UTF-4 value */
        if (out >= outend)
            break;
        if      (c <    0x80) {  *out++=  c;                bits= -6; }
        else if (c <   0x800) {  *out++= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
        else if (c < 0x10000) {  *out++= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
        else                  {  *out++= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }
 
        for ( ; bits >= 0; bits-= 6) {
            if (out >= outend)
                break;
            *out++= ((c >> bits) & 0x3F) | 0x80;
        }
        processed = in;
    }
    *outlen = out - outstart;
    *inlen = processed - in;
    return(0);
}

inline static uint32_t utf8_to_utf16le(uint32_t c) {
    uint8_t chars[5] = {(uint8_t)c, (uint8_t)(c >> 8), (uint8_t)(c >> 16), (uint8_t)(c >> 24), 0};
    int inlen = strlen((const char*)chars);
    uint16_t out[5];
    int outlen = 5;
    if(utf8_to_utf16le(out, &outlen, chars, &inlen) == 0) {
        // printf("%X %X\n", c, out[0]);
        return out[0];
    }
    return 0;
}

inline static uint32_t utf16le_to_utf8(uint32_t c) {
    uint16_t chars[2] = {(uint16_t)c, 0};
    int inlen = 1;
    uint8_t out[8] = {};
    int outlen = 8;
    if(utf16le_to_utf8(out, &outlen, chars, &inlen) == 0) {
        uint32_t r = 0;
        for(uint8_t i = 0; i < 4; ++i) {
            if(out[i] == 0) break;
            r |= ((uint32_t)out[i]) << (8 * i);
        }
        return r;
    }
    return 0;
}
