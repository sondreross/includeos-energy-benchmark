CC = clang
CFLAGS = -Wall -O2 -I../..

OBJS = dijkstra_small.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
