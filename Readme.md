### Raspberry Pi HTTP Proxy Server

This repository contains a simple experimental HTTP proxy server designed for the **Raspberry Pi 3** with **2GB of RAM**, running almost any Linux distribution.

## Initial Notes

- This proxy currently supports only **HTTP GET requests**, as I wanted to implement a caching system with **Least Recently Used (LRU) eviction**. Handling of **HTTPS (CONNECT requests)** will be added in the future, bypassing caching and LRU mechanisms.
- The project is **not yet complete**, and I plan to continue refining and improving it.
- Most importantly: This was built as a learning project. While the implementation may not be the most optimized, the goal was to challenge myself, design a system from scratch, and bring an idea to reality.

## Architecture & Design

![Proxy Server](https://github.com/user-attachments/assets/ae0735d5-3ce1-483e-9fbe-bff303b6bd12)

This proxy server includes a **custom-built cache and LRU system**:

- The **cache** is implemented as a **hash map**, mapping requested resources to their cached responses.
- The **LRU (Least Recently Used) list** is implemented as a **doubly linked list**, tracking access order.
- Each cached resource entry contains:
  - The **HTTP response data**.
  - A **pointer to its position in the LRU list**, allowing for **O(1) eviction** when space is needed.

### Request Handling Flow

1. **Client sends an HTTP GET request** to the proxy.
2. **Proxy receives and processes the request**.
3. **Cache lookup**: Proxy checks if the resource is cached.
4. **Cache validation**: Proxy sends a `HEAD` request to the origin server to check the last modified date.
5. **If the resource is valid in the cache**, return the cached response.
6. **If the resource is stale**, fetch a fresh version, update the cache, and return the response.
7. **If the resource is not cached**, fetch it from the origin server, send it to the client, and store it in the cache.

## Compilation & Usage

1. **Compile the proxy server**:
   ```sh
   make proxy-server
   ```
2. **Run the proxy server**:
   ```sh
   ./proxy-server [SERVER_PORT_NUMBER]
   ```
3. **Test with cURL**:
   ```sh
   curl -x http://[server-ip]:[server-port] http://example.com
   ```
