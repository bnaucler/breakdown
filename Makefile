CC := cc
CFLAGS := `sdl2-config --libs --cflags` -ggdb3 -O0 --std=c99 -Wall -lSDL2 -lSDL2_image -lSDL2_ttf -lm -I/usr/local/include

EXEC := breakdown

# Header files
HDRS := breakdown.h

# Source files
SRCS := breakdown.c

# Generate object names
OBJS := $(SRCS:.c=.o)

# Default
all: $(EXEC)

# Recipe
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)

# Make pretty
clean:
	rm -f $(EXEC) $(OBJS)
