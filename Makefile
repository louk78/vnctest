TARGET= vnctest.exe
CFLAGS=-Wall -Werror -g -O0 -fno-omit-frame-pointer
CC=gcc

CFLAGS+=`yacapi-config --cflags`
LDFLAGS+=`yacapi-config --libs`

all: $(TARGET)

%.exe:	%.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o
