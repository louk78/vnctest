TARGET= vnctest
CFLAGS=-Wall -Werror -g -O0 -fno-omit-frame-pointer
CC=gcc

all: $(TARGET)

clean:
	$(RM) $(TARGET) *.o
