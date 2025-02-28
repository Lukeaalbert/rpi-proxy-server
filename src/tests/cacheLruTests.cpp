#include <gtest/gtest.h>
#include "../cache.hpp"

TEST(CacheTest, InsertAndRetrieve) {
    Cache cache;
    
    cache.insert("http://example.com", "header1", "content1", "lastModified1");

    CacheItem* response = cache.get("http://example.com");

    // ensure response is not null
    ASSERT_NE(response, nullptr);

    ASSERT_EQ(response-> header, "header1");
    ASSERT_EQ(response-> content, "content1");
    ASSERT_EQ(response-> lastModified, "lastModified1");
}

TEST(CacheTest, InsertRetrieveMany) {
    Cache cache;
    
    cache.insert("http://example1.com", "header1", "content1", "lastModified1");
    cache.insert("http://example2.com", "header2", "content2", "lastModified2");
    cache.insert("http://example3.com", "header3", "content3", "lastModified3");
    cache.insert("http://example4.com", "header4", "content4", "lastModified4");

    CacheItem* response = cache.get("http://example3.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header3");
    ASSERT_EQ(response-> content, "content3");
    ASSERT_EQ(response-> lastModified, "lastModified3");

    response = cache.get("http://example1.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header1");
    ASSERT_EQ(response-> content, "content1");
    ASSERT_EQ(response-> lastModified, "lastModified1");

    response = cache.get("http://example2.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header2");
    ASSERT_EQ(response-> content, "content2");
    ASSERT_EQ(response-> lastModified, "lastModified2");
}

TEST(CacheTest, InsertRetrieveAndRemoveMany) {
    Cache cache;
    
    cache.insert("http://example1.com", "header1", "content1", "lastModified1");
    cache.insert("http://example2.com", "header2", "content2", "lastModified2");
    cache.insert("http://example3.com", "header3", "content3", "lastModified3");
    cache.insert("http://example4.com", "header4", "content4", "lastModified4");

    CacheItem* response = cache.get("http://example3.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header3");
    ASSERT_EQ(response-> content, "content3");
    ASSERT_EQ(response-> lastModified, "lastModified3");

    response = cache.get("http://example1.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header1");
    ASSERT_EQ(response-> content, "content1");
    ASSERT_EQ(response-> lastModified, "lastModified1");

    cache.remove(2);

    response = cache.get("http://example4.com");
    ASSERT_EQ(response, nullptr);

    response = cache.get("http://example2.com");
    ASSERT_EQ(response, nullptr);

    response = cache.get("http://example1.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header1");
    ASSERT_EQ(response-> content, "content1");
    ASSERT_EQ(response-> lastModified, "lastModified1");

    response = cache.get("http://example3.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header3");
    ASSERT_EQ(response-> content, "content3");
    ASSERT_EQ(response-> lastModified, "lastModified3");

    cache.remove(1);

    response = cache.get("http://example2.com");
    ASSERT_EQ(response, nullptr);

    response = cache.get("http://example3.com");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header3");
    ASSERT_EQ(response-> content, "content3");
    ASSERT_EQ(response-> lastModified, "lastModified3");
}

TEST(CacheTest, InsertMaxAndRemoveMany) {
    Cache cache;

    // 454 = max cache size
    for (size_t i = 0; i < 455; i++) {
        cache.insert("http://example.com" + std::to_string(i), "header" + std::to_string(i),
            "content" + std::to_string(i), "lastModified" + std::to_string(i));
    }

    // should resize and delete 150 first entries, resulting in
    // the front being item 150.
    CacheItem* response = cache.get("http://example.com150");

    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response-> header, "header150");
    ASSERT_EQ(response-> content, "content150");
    ASSERT_EQ(response-> lastModified, "lastModified150");

    cache.remove(1);

    response = cache.get("http://example.com151");
    ASSERT_EQ(response, nullptr);
}

TEST(LruTest, InsertPushPopRemoveSimple) {
    LRU lru;

    lru.push("http://example1.com");
    lru.push("http://example2.com");

    LruEntry* front = lru.front();
    LruEntry* back = lru.back();

    ASSERT_NE(front, nullptr);
    ASSERT_EQ(front-> uri, "http://example1.com");
    ASSERT_EQ(front-> next, back);
    ASSERT_EQ(front-> prev, nullptr);

    ASSERT_NE(back, nullptr);
    ASSERT_EQ(back-> uri, "http://example2.com");
    ASSERT_EQ(back-> next, nullptr);
    ASSERT_EQ(back-> prev, front);

    lru.remove(back);

    back = lru.back();

    ASSERT_NE(back, nullptr);
    ASSERT_EQ(front, back);

    lru.pop();

    ASSERT_TRUE(lru.isEmpty());
}

TEST(LruTest, InsertPushPopRemoveMany) {
    LRU lru;

    lru.push("http://example1.com");
    lru.push("http://example2.com");
    lru.push("http://example3.com");
    lru.push("http://example4.com");

    ASSERT_EQ(lru.size(), 4);

    lru.pop();

    LruEntry* front = lru.front();
    ASSERT_NE(front, nullptr);
    ASSERT_EQ(front-> uri, "http://example2.com");
    ASSERT_EQ(front-> prev, nullptr);

    LruEntry* back = lru.back();
    lru.remove(back);
    lru.pop();

    front = lru.front();

    ASSERT_NE(front, nullptr);
    ASSERT_EQ(front-> uri, "http://example3.com");
    ASSERT_EQ(front-> next, nullptr);
    ASSERT_EQ(front-> prev, nullptr);

    lru.pop();

    ASSERT_TRUE(lru.isEmpty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
