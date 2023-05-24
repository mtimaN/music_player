
CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -I/usr/local/include/SDL2 -I/usr/X11R6/include -L/usr/local/lib
LIBS = `pkg-config --libs gtk+-3.0` -lSDL2
build: main

main: main.o audio.o
	$(CC) $(CFLAGS) $(SDL) -o main main.o audio.o $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -f *.o main