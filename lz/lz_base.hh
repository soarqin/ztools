#pragma once

class lz_base {
public:
    virtual ~lz_base() {}
    virtual bool detect(FILE*) = 0;
    virtual void decompress(FILE*, FILE*) = 0;
    virtual void compress(FILE*, FILE*) = 0;
    virtual void set_compress_option(uint32_t index, uint32_t value) {}
};
