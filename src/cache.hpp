#ifndef CACHE_HPP
#define CACHE_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>

#include "LRU.hpp"

class Cache {
    
    public:

    void Cache::insert(const std::string& url, const std::string& header, const std::string& content, 
        const std::string& lastModified) {
        auto it = mCache.find(url);
        if (it != mCache.end()) {
            return;
        }
        std::unique_ptr<CachedHttpResponse> cachedResponse = 
            std::make_unique<CachedHttpResponse>(header, content, lastModified);
        mLru.push(cachedResponse.get());
        mCache.emplace(url, std::move(cachedResponse));
    }   

    CachedHttpResponse* Cache::get(const std::string& url) {
        auto it = mCache.find(url);
        if (it != mCache.end()) {
            mLru.remove(it -> second.get());
            mLru.push(it -> second.get());
            return it -> second.get();
        }
        return nullptr;
    }

    void remove(size_t amountToRemove) {
        for (size_t i = 0; i < amountToRemove; ++i) {
            mLru.pop();
        }
    }

    void clear() {
        while (!mLru.isEmpty()) {
            mLru.pop();
        }
    }

    private:

        std::unordered_map<std::string, std::unique_ptr<CachedHttpResponse> > mCache;
        LRU mLru;
};

#endif //CACHE_HPP