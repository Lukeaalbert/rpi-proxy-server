#include <iostream>
#include <random>
#include <string>

#include "cache.h"

namespace {

    long getRandomNumberInRange(const long& randNum) {
        // obtain a random number from hardware
        std::random_device rd;
        // seed the generator
        std::mt19937 gen(rd());
        // define the range
        std::uniform_int_distribution<> distr(0, randNum);

        return distr(gen);
    }
}

Cache::Cache(long cacheStaleness): mCacheStaleness(cacheStaleness) {}

std::string Cache::get(const std::string& url) {
    auto it = mCache.find(url);
    if (it != mCache.end()) {
        return it -> second.content;
    }
    return std::string();
}

void Cache::refresh() {
    auto it = mCache.begin();
    while (it != mCache.end()) {
        if (it -> second.staleness == mCacheStaleness) {
            mCache.erase(it);
        }
        else {
            it -> second.staleness++;
            it++;
        }
    }
}

void Cache::insert(const std::string& url, const std::string& content) {
    CachedHttpResponse chr(content);
    mCache.emplace(url, chr);
    mCache[url].staleness = 0;
}