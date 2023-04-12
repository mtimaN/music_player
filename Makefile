CC=gcc

main: main.c
	$(CC) main.c -I/usr/local/include/SDL2 -I/usr/X11R6/include -L/usr/local/lib -lSDL2 -o audioplayer

.PHONY: clean
# Target to clean files created during compilation
clean:
	rm -f *.o audioplayer