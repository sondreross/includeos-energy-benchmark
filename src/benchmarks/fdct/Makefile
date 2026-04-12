CC = clang
CFLAGS = -Wall -O2 -I../..

OBJS = libfdct.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
