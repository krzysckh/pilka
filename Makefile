CFLAGS=-std=c99 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=2
LDFLAGS=-lraylib -lm

OFILES=pilka.o

all: pilka
.c.o:
	$(CC) $(CFLAGS) -c $<
pilka: $(OFILES)
	$(CC) -o pilka $(OFILES) $(LDFLAGS)
clean:
	rm -f pilka *.o
