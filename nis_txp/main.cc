#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[]) {
    FILE* f = fopen(argv[1], "rb");
    if (f == nullptr) return -1;
    uint16_t w, h;
    fread(&w, 2, 1, f);
    fread(&h, 2, 1, f);
    uint32_t* pixels = new uint32_t[w * h];
    uint32_t bw = 32, bh = 8;
    uint32_t palette[0x100];
    fseek(f, 0x10, SEEK_SET);
    fread(palette, 4, 0x100, f);
    uint32_t sz = w * h / 2;
    uint8_t* data = new uint8_t[sz];
    fread(data, 1, sz, f);
    fclose(f);
    uint32_t cx = 0, cy = 0;
    for(uint32_t i = 0; i < sz; ++i) {
        uint8_t c = data[i];
        pixels[w * cy + cx] = palette[c & 0x0F];
        pixels[w * cy + cx + 1] = palette[c >> 4];
        cx += 2;
        if ((cx & 0x1F) == 0) {
            ++ cy;
            if ((cy & 0x07) == 0) {
                if (cx >= w) {
                    cx = 0;
                } else {
                    cy -= bh;
                }
            } else {
                cx -= bw;
            }
        }
    }
    delete[] data;
    stbi_write_png(argv[2], w, h, 4, pixels, 0);
    delete[] pixels;
    return 0;
}
