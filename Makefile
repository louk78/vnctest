TARGET= vnctest.exe main.exe
CFLAGS=-Wall -Werror -g -O0 -fno-omit-frame-pointer
CC=gcc

ifneq ($(strip $(shell $(CC) -v 2>&1 | grep "mingw")),)
OS_WIN32=true
endif

ifdef OS_WIN32
CFLAGS+=`yacapi-config --cflags`
LDFLAGS+=`yacapi-config --libs`
endif

all: $(TARGET)

main.o: main.c vnc.h vnc_proto.h
vnc.o: vnc.c vnc.h vnc_proto.h

main.exe: main.o vnc.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.exe:	%.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o
