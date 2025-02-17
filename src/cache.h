#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <string>
#include <memory>

struct CachedHttpResponse {
    CachedHttpResponse(const std::string h, const std::string c, const std::string lm):
        header(h), content(c), lastModified(lm) {}
    std::string header;
    std::string content;
    std::string lastModified;
};

class Cache {
    public:

    void insert(const std::string& url, const std::string& header, const std::string& content,
        const std::string& lastModified);

    CachedHttpResponse* get(const std::string& url);

    void remove(size_t amountToRemove);

    void clear();

    private:

        std::unordered_map<std::string, std::unique_ptr<CachedHttpResponse> > mCache;
};

#endif //CACHE_H