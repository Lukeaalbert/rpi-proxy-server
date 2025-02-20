MY_DEFS = -std=c++17

# google test stuff
G_TEST_INCLUDES = -I googletest/googletest/include
G_TEST_LIBS = googletest/build/lib/libgtest.a googletest/build/lib/libgtest_main.a

cache-lru-tests: tests/cacheLruTests.cpp
	g++ ${MY_DEFS} -pthread ${G_TEST_INCLUDES} -o cache-lru-tests tests/cacheLruTests.cpp $(G_TEST_LIBS)
	# Run the tests
	./cache-lru-tests

proxy-server: main.cpp server.h server.cpp
	g++ ${MY_DEFS} -o proxy-server main.cpp server.cpp

clean:
	-rm -f proxy-server cache-lru-tests server.o cache.o
