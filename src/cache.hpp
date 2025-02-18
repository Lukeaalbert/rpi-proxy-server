#ifndef CACHE_HPP
#define CACHE_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>

#include "cachedResponse.h"

class Cache {
    
    public:

    void insert(const std::string& url, const std::string& header, const std::string& content, 
        const std::string& lastModified) {
        
        // **********************
        // *** IMPORTANT NOTE *** 
        // **********************
        // 454 is an arbitrary number that was picked for the
        // rpi 3 with 2GB of RAM. I assumed each cached entry 
        // costed around 2.2MB and wanted to keep the cache under
        // 1GB, and 2.2MB * 454 = about 1GB.
        // Change at your discretion. 
        if (mLru.size() >= 454) {
            remove(150);
        }

        auto it = mCache.find(url);
        if (it != mCache.end()) {
            return;
        }
        std::unique_ptr<CachedHttpResponse> cachedResponse = 
            std::make_unique<CachedHttpResponse>(header, content, lastModified);
        mLru.push(url);
        cachedResponse -> lruEntry =  mLru.tail();
        mCache.emplace(url, std::move(cachedResponse));
    }

    CachedHttpResponse* get(const std::string& url) {
        auto it = mCache.find(url);
        if (it != mCache.end()) {
            mLru.remove(it -> second -> lruEntry);
            mLru.push(url);
            it -> second -> lruEntry = mLru.tail();
            return it -> second.get();
        }
        return nullptr;
    }

    void remove(size_t amountToRemove) {
        for (int i = 0; i < amountToRemove; i++) {
            LruEntry* toRemove = mLru.front();
            mCache.erase(toRemove -> uri);
            mLru.pop();
        }
    }

    private:

        std::unordered_map<std::string, std::unique_ptr<CachedHttpResponse> > mCache;
        LRU mLru;
};

#endif //CACHE_HPP