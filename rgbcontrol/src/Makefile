CC = gcc
CFLAGS = -Wall
OBJS = rgbcontrol.o

all: rgbcontrol

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

rgbcontrol: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f rgbcontrol *.o
