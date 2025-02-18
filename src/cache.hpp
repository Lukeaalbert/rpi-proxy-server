#ifndef CACHE_HPP
#define CACHE_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>

#include "LRU.hpp"

struct CacheItem {
    CacheItem(const std::string h, const std::string c, const std::string lm):
       header(h), content(c), lastModified(lm) {}
    
    // HTTP header, content, and last modified date
    std::string header;
    std::string content;
    std::string lastModified;

    // pointer to entry in lru
    LruEntry* lruEntry = nullptr;
};

class Cache {
    
    public:

    void insert(const std::string& url, const std::string& header, const std::string& content, 
        const std::string& lastModified) {
        // return if item already in cache
        auto it = mCache.find(url);
        if (it != mCache.end()) {
            return;
        }
        
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

        // create cache item
        std::unique_ptr<CacheItem> cachedResponse = 
            std::make_unique<CacheItem>(header, content, lastModified);
        mLru.push(url); // put item in LRU
        cachedResponse -> lruEntry =  mLru.back(); // Connect cache item to point to LRU spot
        mCache.emplace(url, std::move(cachedResponse)); // put cache item in cache
    }

    CacheItem* get(const std::string& url) {
        auto it = mCache.find(url); // find cache item in cache
        if (it != mCache.end()) { // if found
            // remove and push back into cache
            // so that it's in the very back
            mLru.remove(it -> second -> lruEntry);
            mLru.push(url);
            // necesary update to cache item's pointer to LRU item
            it -> second -> lruEntry = mLru.back();
            return it -> second.get(); // return pointer to cache item 
        }
        return nullptr; // return null if not found
    }

    void remove(size_t amountToRemove) {
        for (int i = 0; i < amountToRemove; i++) {
            mCache.erase(mLru.front() -> uri); // erase from cache
            mLru.pop(); // erase from lru
        }
    }

    private:
        std::unordered_map<std::string, std::unique_ptr<CacheItem> > mCache;
        LRU mLru;
};

#endif //CACHE_HPP