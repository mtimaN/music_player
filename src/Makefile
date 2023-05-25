
CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` `sdl2-config --cflags` -g
LIBS = `pkg-config --libs gtk+-3.0` -lSDL2 -lSDL2_mixer

# Specify the target file to build
TARGET = main

# List of object files needed for the target
OBJS = main.o audio.o

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Rule to compile C source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to clean the compiled files
clean:
	rm -f $(OBJS) $(TARGET)