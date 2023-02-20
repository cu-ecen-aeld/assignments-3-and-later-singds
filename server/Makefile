CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc
CFLAGS=
INCLUDES=
LDFLAGS=

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) $(INCLUDES) aesdsocket.c -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f writer
