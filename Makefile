CFLAGS=-std=c99 -Wall -Wextra -O2
LDFLAGS=-lraylib -lm

OFILES=pilka.o

all: pilka
.c.o:
	$(CC) $(CFLAGS) -c $<
pilka: $(OFILES)
	$(CC) -o pilka $(OFILES) $(LDFLAGS)
clean:
	rm -f pilka *.o
