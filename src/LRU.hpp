#ifndef LRU_HPP
#define LRU_HPP

#include "cachedResponse.h" 

class LRU {

    public:

    LRU(): mHead(nullptr), mTail(nullptr), mSize(0) {}

    void push(CachedHttpResponse* data) {
        // empty lru
        if (mHead == nullptr && mTail == nullptr) {
            mHead = data;
            mTail = data;
            ++mSize;
        }
        else { // non empty lru
            mTail -> nextInLru = data;
            mTail = data;
            ++mSize;
        }
    }

    CachedHttpResponse* pop() {
        // empty lru
        if (mHead == nullptr || mTail == nullptr) {
            return nullptr;
        }
        else { // non empty lru
            CachedHttpResponse* toReturn = mHead;
            mHead = mHead -> nextInLru;
            if (mHead == nullptr) {
                mTail = nullptr;
            } else {
                mHead -> prevInLru = nullptr;
            }
            --mSize;
            return toReturn;
        }
    }

    CachedHttpResponse* front() {
        // empty lru
        if (mHead == nullptr || mTail == nullptr) {
            return nullptr;
        }
        else { // non empty lru
            return mHead;
        }
    }

    void remove(CachedHttpResponse* data) {
        // empty lru
        if (mHead == nullptr || mTail == nullptr) {
            return;
        }
        else if (data == mHead) {
            mHead = mHead -> nextInLru;
            if (mHead != nullptr) {
                mHead -> prevInLru = nullptr;
            } else {
                mTail = nullptr;
            }
            --mSize;
        }
        else if (data == mTail) {
            mTail = mTail -> prevInLru;
            if (mTail != nullptr) {
                mTail -> nextInLru = nullptr;
            }
            --mSize;
        }
        else {
            data -> prevInLru -> nextInLru = data -> nextInLru;
            data -> nextInLru -> prevInLru = data -> prevInLru;
            --mSize;
        }
    }

    bool isEmpty() {
        return (mHead == nullptr && mTail == nullptr);
    }

    size_t size() {
        return mSize;
    }

    private:
        CachedHttpResponse* mHead;
        CachedHttpResponse* mTail;
        size_t mSize;
};

#endif // LRU_HPP
