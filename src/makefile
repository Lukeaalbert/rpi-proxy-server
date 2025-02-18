MY_DEFS = -std=c++17

# google test stuff
G_TEST_INCLUDES = -I googletest/googletest/include
G_TEST_LIBS = googletest/build/lib/libgtest.a googletest/build/lib/libgtest_main.a

cache-tests: tests/cacheTests.cpp
	g++ ${MY_DEFS} -pthread ${G_TEST_INCLUDES} -o cache-tests tests/cacheTests.cpp $(G_TEST_LIBS)
	# Run the tests
	./cache-tests

proxy-server: main.cpp server.h server.cpp cache.hpp
	g++ ${MY_DEFS} -o proxy-server main.cpp server.cpp cache.hpp

clean:
	-rm -f proxy-server cache-tests server.o cache.o
