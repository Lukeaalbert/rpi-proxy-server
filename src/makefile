MY_DEFS = -std=c++14

server: server.h server.cpp cache.h cache.cpp
	g++ ${MY_DEFS} -o server server.cpp cache.cpp

cache: cache.h cache.cpp
	g++ ${MY_DEFS} -o cache cache.cpp

clean:
	-rm -f server cache server.o cache.o