CFLAGS=-std=c99 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=2 #-pg -ggdb
LDFLAGS=-lraylib -lm #-pg -ggdb

OFILES=pilka.o

all: pilka
.c.o:
	$(CC) $(CFLAGS) -c $<
pilka: $(OFILES)
	$(CC) -o pilka $(OFILES) $(LDFLAGS)
pilka.exe: libraylib-w32.a
	$(MAKE) CC=i686-w64-mingw32-gcc LDFLAGS="-L. -l:libraylib-w32.a -lm -lwinmm -lgdi32 -static" CFLAGS="$(CFLAGS) -mwindows -I/usr/local/include" pilka
libraylib-w32.a:
	wget -O $@ "https://pub.krzysckh.org/raylib-bin/libraylib5-win32-opengl33.a"
clean:
	rm -f pilka pilka.exe *.o
