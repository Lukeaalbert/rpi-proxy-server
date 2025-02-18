#ifndef LRU_HPP
#define LRU_HPP

struct LruEntry {
    LruEntry(std::string u, LruEntry* p, LruEntry* n):
        uri(u), next(n), prev(p) {}
    std::string uri;
    LruEntry* next;
    LruEntry* prev;
};

class LRU {

    public:

    LRU(): mHead(nullptr), mTail(nullptr), mSize(0) {}

    ~LRU() {
        LruEntry* curr = mHead;
        while (curr != nullptr) {
            LruEntry* next = curr -> next;
            delete curr;
            curr = next;
        }
    }

    void push(std::string uri) {
        // empty lru
        if (mHead == nullptr && mTail == nullptr) {
            LruEntry* newEntry = new LruEntry(uri, nullptr, nullptr);
            mHead = newEntry;
            mTail = newEntry;
            ++mSize;
        }
        else { // non empty lru
            LruEntry* newEntry = new LruEntry(uri, mTail, nullptr);
            mTail -> next = newEntry;
            mTail = newEntry;
            ++mSize;
        }
    }

    void pop() {
        // empty lru
        if (mHead == nullptr || mTail == nullptr) {
            return;
        }
        else { // non empty lru
            LruEntry* toDelete = mHead;
            mHead = mHead -> next;
            if (mHead == nullptr) {
                mTail = nullptr;
            } else {
                mHead -> prev = nullptr;
            }
            --mSize;
            delete toDelete;
        }
    }

    void remove(LruEntry* data) {
        // empty lru
        if (mHead == nullptr || mTail == nullptr) {
            return;
        }
        else if (data == mHead) {
            mHead = mHead -> next;
            if (mHead != nullptr) {
                mHead -> prev = nullptr;
            } else {
                mTail = nullptr;
            }
            --mSize;
        }
        else if (data == mTail) {
            mTail = mTail -> prev;
            if (mTail != nullptr) {
                mTail -> next = nullptr;
            }
            --mSize;
        }
        else {
            data -> prev -> next = data -> next;
            data -> next -> prev = data -> prev;
            --mSize;
        }
        delete data;
    }

    size_t size() {
        return mSize;
    }

    LruEntry* front() {
        return mHead;
    }

    LruEntry* back(){
        return mTail;
    }
    
    bool isEmpty() {
        return (mHead == nullptr && mTail == nullptr);
    }

    private:
        LruEntry* mHead;
        LruEntry* mTail;
        size_t mSize;
};

#endif // LRU_HPP
