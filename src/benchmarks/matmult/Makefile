CC = clang
CFLAGS = -Wall -O2 -I../.. -DMATMULT_INT

OBJS = matmult.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean
