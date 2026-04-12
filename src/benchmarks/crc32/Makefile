CC = clang
CFLAGS = -Wall -O2 -I../..

OBJS = crc_32.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
