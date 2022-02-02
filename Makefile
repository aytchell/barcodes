BASIC_CFLAGS = -Wall -pedantic 
LIBEV_CFLAGS = `pkg-config --cflags libevdev`
CFLAGS = $(BASIC_CFLAGS) $(LIBEV_CFLAGS)

LDFLAGS = `pkg-config --libs libevdev`

all: key_event list

input_device.o: input_device.h input_device.c
	gcc $(CFLAGS) -c input_device.c

input_buffer.o: input_buffer.h input_buffer.c
	gcc $(CFLAGS) -c input_buffer.c

list.o: list.c input_device.h
	gcc $(CFLAGS) -c list.c

key_event.o: key_event.c input_buffer.h
	gcc $(CFLAGS) -c key_event.c

key_event: key_event.o input_buffer.o input_device.o
	gcc key_event.o input_buffer.o input_device.o -o key_event $(LDFLAGS)

list: list.o input_device.o
	gcc list.o input_device.o -o list $(LDFLAGS)


.PHONY: clean
clean:
	rm -f *.o list key_event
