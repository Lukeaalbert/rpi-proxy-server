#ifndef LRU_HPP
#define LRU_HPP

#include "cachedResponse.h" 

class LRU {

    public:

    LRU(): head(nullptr), tail(nullptr) {}

    void push(CachedHttpResponse* data) {
        // empty lru
        if (head == nullptr && tail == nullptr) {
            head = data;
            tail = data;
        }
        else { // non empty lru
            tail -> nextInLru = data;
            tail = data;
        }
    }

    CachedHttpResponse* pop() {
        // empty lru
        if (head == nullptr || tail == nullptr) {
            return nullptr;
        }
        else { // non empty lru
            CachedHttpResponse* toReturn = head;
            head = head -> nextInLru;
            if (head == nullptr) {
                tail = nullptr;
            } else {
                head -> prevInLru = nullptr;
            }
            return toReturn;
        }
    }

    void remove(CachedHttpResponse* data) {
        // empty lru
        if (head == nullptr || tail == nullptr) {
            return;
        }
        else if (data == head) {
            head = head -> nextInLru;
            if (head != nullptr) {
                head -> prevInLru = nullptr;
            } else {
                tail = nullptr;
            }
        }
        else if (data == tail) {
            tail = tail -> prevInLru;
            if (tail != nullptr) {
                tail -> nextInLru = nullptr;
            }
        }
        else {
            data -> prevInLru -> nextInLru = data -> nextInLru;
            data -> nextInLru -> prevInLru = data -> prevInLru;
        }
    }

    bool isEmpty() {
        return (head == nullptr && tail == nullptr);
    }

    private:
        CachedHttpResponse* head;
        CachedHttpResponse* tail;
};

#endif // LRU_HPP
