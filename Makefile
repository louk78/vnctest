TARGET= vnctest.exe
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

%.exe:	%.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o
