CC = clang
CFLAGS = -Wall -O2 -I../..

OBJS = aesxam.o aes.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
