#pragma once

#include "lz_base.hh"

class lzs: public lz_base {
public:
    static void reg();
    virtual ~lzs() {}
    bool detect(FILE*) override;
    void decompress(FILE*, FILE*) override;
    void compress(FILE*, FILE*) override;
    void set_compress_option(uint32_t index, uint32_t value) override;

private:
    uint32_t file_ext = 0x746164;
    uint8_t comp_flag = 0xD5;
};
