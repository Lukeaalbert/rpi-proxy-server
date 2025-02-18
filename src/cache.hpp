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
        
        // **********************
        // *** IMPORTANT NOTE *** 
        // **********************
        // 454 is an arbitrary number that was picked for the
        // rpi 3 with 2GB of RAM. I assumed each cached entry 
        // costed around 2.2MB and wanted to keep the cache under
        // 1GB, and 2.2MB * 454 = about 1GB.
        // Change at your discretion. 
        if (mLru.size() >= 454) {
            // TODO: erase some amount from cache and lru 
        }

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
        // TODO
    }

    void clear() {
       // TODO
    }

    private:

        std::unordered_map<std::string, std::unique_ptr<CachedHttpResponse> > mCache;
        LRU mLru;
};

#endif //CACHE_HPP