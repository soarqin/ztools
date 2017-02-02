#pragma once

#include "lz_base.hh"
#include <map>

class lz {
public:
    ~lz();
    lz_base* detect(FILE*);
    lz_base* get(const char* k);
    inline void reg(const char* k, lz_base* b) {
        lz_map[str_to_key(k)] = b;
    }

private:
    uint32_t str_to_key(const char* s);

private:
    std::map<uint32_t, lz_base*> lz_map;
};

extern lz g_lz;
