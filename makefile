all:
	g++ -O3 -pedantic -Wall -Wextra -o marmer marmer.cpp $$(libpng-config --cflags --ldflags) -I$(HOME)/usr/include
