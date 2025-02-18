#ifndef CACHEDRESPONSE_H
#define CACHEDRESPONSE_H
#include <string>

struct CachedHttpResponse {
    CachedHttpResponse(const std::string h, const std::string c, const std::string lm):
        header(h), content(c), lastModified(lm) {}
    std::string header;
    std::string content;
    std::string lastModified;

    CachedHttpResponse* nextInLru = nullptr;
    CachedHttpResponse* prevInLru = nullptr;
};

#endif // CACHEDRESPONSE_H