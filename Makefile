BASIC_CFLAGS = -Wall -pedantic 
LIBEV_CFLAGS = `pkg-config --cflags libevdev`
CFLAGS = $(BASIC_CFLAGS) $(LIBEV_CFLAGS)

LDFLAGS = `pkg-config --libs libevdev`

key_event: key_event.c input_buffer.c input_buffer.h
	gcc $(CFLAGS) key_event.c input_buffer.c -o key_event $(LDFLAGS)
