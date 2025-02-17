MY_DEFS = -std=c++14

proxy-server: main.cpp server.h server.cpp cache.h cache.cpp
	g++ ${MY_DEFS} -o proxy-server main.cpp server.cpp cache.cpp

cache: cache.h cache.cpp
	g++ ${MY_DEFS} -o cache cache.cpp

clean:
	-rm -f proxy-server cache server.o cache.o