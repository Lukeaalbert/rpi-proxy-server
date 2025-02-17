#include <iostream>
#include <random>
#include <string>
#include <memory>

#include "cache.h"

CachedHttpResponse* Cache::get(const std::string& url) {
    auto it = mCache.find(url);
    if (it != mCache.end()) {
        return it -> second.get();
    }
    return nullptr;
}

void Cache::insert(const std::string& url, const std::string& header, const std::string& content, 
    const std::string& lastModified) {
    std::unique_ptr<CachedHttpResponse> cachedResponse = 
        std::make_unique<CachedHttpResponse>(header, content, lastModified);
    mCache.emplace(url, std::move(cachedResponse));
}

void Cache::remove(size_t amountToRemove) {}

void Cache::clear() {}