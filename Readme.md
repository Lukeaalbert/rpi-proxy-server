### Raspberry Pi HTTP Proxy Server
I wrote this simple experimental proxy server designed specifically for the Raspberry Pi 3 w/2GB of RAM running almost any flavor or Linux. Some important initial notes:
* Because I wanted to implement a cache and LRU (more on these below), this server is currently only working with HTTP (not HTTPS) GET requests. I'll fix this later, implementing the handling of HTTPS CONNECT requests and skipping over the cache and LRU as a whole, but I really wanted to include these in the first part of this project.
* This is **not complete** and I will likely continue to revise and add to it over the foreseeable future.
* Lastly, but most importantly: I know this is likely a naive and not optimal implementation. I built this with almost no knowledge of how proxy servers actually work, and **the motivation for this project was not to build the most perfect product, but to learn and prove to myself that I can be an engineer who imagines an original design and brings it to reality.**

<img width="1072" alt="proxy" src="https://github.com/user-attachments/assets/ae0735d5-3ce1-483e-9fbe-bff303b6bd12" />

As aforementioned, I created a custom cache and LRU from scratch for the server. I decided to stick with a hash map for the underlying data structure of the cache and a doubly linked list for the underlying data structure of the LRU. In addition to storing a resource's cached HTTP response data, the cache hash map also stores a pointer to the node representing that resource in the LRU. This allows for O(1) removal from the LRU, which was important to me because data is removed and re-pushed into the LRU every time it's accessed.

The general flow I designed for this server was:
1. User makes a GET request.
2. Server receives and processes request.
3. Server checks if the resource is in the cache and sends a HEAD request to the resource to find the last modified date.
4. If the resource is in the cache and it's not stale, the server sends the cached resource back to the client.
5. If the resource is in the cache but it is stale, the server makes a GET request to the resource, sends the resource to the client, and updates the cache info.
6. Otherwise, the server makes a GET request to the resource, sends the resource to the client, and inserts the data into the cache.

Compile with **make proxy-server** and run with **./proxy-server [SERVER PORT NUMBER]**.

Test with **curl -x http:/[server-ip]:[server port number] http://example.com**.
