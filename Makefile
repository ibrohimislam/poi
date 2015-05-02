all: main.cc poi.o volume.o entry.o
	g++ main.cc poi.o volume.o entry.o -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs` -o mount-poi

poi.o: poi.h poi.cc
	g++ -Wall -c poi.cc -D_FILE_OFFSET_BITS=64

volume.o: volume.h volume.cc
	g++ -Wall -c volume.cc -D_FILE_OFFSET_BITS=64

entry.o: entry.h entry.cc
	g++ -Wall -c entry.cc -D_FILE_OFFSET_BITS=64

clean:
	rm *~
	
clear:
	rm *.o
