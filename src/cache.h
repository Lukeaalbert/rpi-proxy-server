#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <string>
#include <memory>

class Cache {
    public:

    Cache(long cacheStaleness);

    void insert(const std::string& url, const std::string& content);

    void randomRemove();

    // void refresh();

    std::string get(const std::string& url);

    private:

        struct CachedHttpResponse {
            CachedHttpResponse(const std::string c): content(c), staleness(0) {}
            std::string content;
            int staleness;
        };

        long mCacheStaleness;
        std::unordered_map<std::string, std::unique_ptr<CachedHttpResponse> > mCache;
};

#endif //CACHE_H