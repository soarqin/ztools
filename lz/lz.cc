#include "pch.hh"

#include "lz.hh"

lz g_lz;

lz::~lz() {
    for(auto& p: lz_map)
        delete p.second;
    lz_map.clear();
}

lz_base* lz::detect(FILE* ifs) {
    for(auto& p: lz_map) {
        if (p.second->detect(ifs))
            return p.second;
    }
    return nullptr;
}

lz_base* lz::get(const char* k) {
    auto ite = lz_map.find(str_to_key(k));
    if (ite == lz_map.end()) return nullptr;
    return ite->second;
}

uint32_t lz::str_to_key(const char* s) {
    uint32_t r = 0;
    for (int i = 0; i < 3 && s[i] != 0; ++i) {
        r |= ((uint32_t)s[i]) << (i * 8);
    }
    return r;
}
