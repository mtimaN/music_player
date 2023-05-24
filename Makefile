CC=gcc
LIBS = -I/usr/local/include/SDL2 -I/usr/X11R6/include -L/usr/local/lib
main: main.c
	$(CC) main.c $(LIBS) -lSDL2 -o audioplayer

.PHONY: clean
# Target to clean files created during compilation
clean:
	rm -f *.o audioplayer