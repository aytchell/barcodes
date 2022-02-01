BASIC_CFLAGS = -Wall -pedantic 
LIBEV_CFLAGS = `pkg-config --cflags libevdev`
CFLAGS = $(BASIC_CFLAGS) $(LIBEV_CFLAGS)

LDFLAGS = `pkg-config --libs libevdev`

key_event: key_event.c
	gcc $(CFLAGS) key_event.c -o key_event $(LDFLAGS)
