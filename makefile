all:
	g++ -O3 -pedantic -Wall -Wextra -o marmer marmer.cpp $$(libpng-config --cflags --ldflags) -ljpeg -I$(HOME)/usr/include -I/usr/local/include
