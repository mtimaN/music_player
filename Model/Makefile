
CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`

build: startGtkApp

start: start.c
	gcc `pkg-config --cflags gtk+-3.0` -o start start.c `pkg-config --libs gtk+-3.0`

startGtkApp: startGtkApp.c
	$(CC) $(CFLAGS) -o startGtkApp startGtkApp.c $(LIBS)

container: container.c
	$(CC) $(CFLAGS) -o container container.c $(LIBS)

box: box.c
	$(CC) $(CFLAGS) -o box box.c $(LIBS)

fixed: fixed.c
	$(CC) $(CFLAGS) -o fixed fixed.c $(LIBS)

clean:
	rm -f start startGtkApp container box fixed