#ifndef CACHEDRESPONSE_H
#define CACHEDRESPONSE_H
#include <string>

#include "LRU.hpp"

struct CachedHttpResponse {
    CachedHttpResponse(const std::string h, const std::string c, const std::string lm):
       header(h), content(c), lastModified(lm) {}

    std::string header;
    std::string content;
    std::string lastModified;

    // pointer to entry in lru
    LruEntry* lruEntry = nullptr;
};

#endif // CACHEDRESPONSE_H