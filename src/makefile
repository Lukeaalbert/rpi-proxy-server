MY_DEFS = -std=c++14

proxy-server: main.cpp server.h server.cpp cache.hpp
	g++ ${MY_DEFS} -o proxy-server main.cpp server.cpp cache.hpp

clean:
	-rm -f proxy-server server.o cache.o