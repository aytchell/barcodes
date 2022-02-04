BASIC_CFLAGS = -Wall -pedantic -Wextra

LIBEV_CFLAGS = `pkg-config --cflags libevdev`
LIBEV_LDFLAGS = `pkg-config --libs libevdev`

LIBCURL_CFLAGS = `pkg-config --cflags libcurl`
LIBCURL_LDFLAGS = `pkg-config --libs libcurl`

CFLAGS = $(BASIC_CFLAGS) $(LIBEV_CFLAGS) $(LIBCURL_CFLAGS)

LDFLAGS = $(LIBEV_LDFLAGS) $(LIBCURL_LDFLAGS)

KEY_EVENT_OBJECTS = key_event.o input_buffer.o input_device.o \
					send_http.o config.o

all: key_event cfg_reader

config.o: config.h config.c
	gcc $(CFLAGS) -c config.c

logger.o: logger.h logger.c config.h
	gcc $(CFLAGS) -c logger.c

input_buffer.o: input_buffer.h input_buffer.c
	gcc $(CFLAGS) -c input_buffer.c

input_device.o: input_device.h input_device.c
	gcc $(CFLAGS) -c input_device.c

send_http.o: send_http.h send_http.c input_buffer.h config.h
	gcc $(CFLAGS) -c send_http.c

key_event.o: key_event.c input_buffer.h input_device.h send_http.h config.h
	gcc $(CFLAGS) -c key_event.c

key_event: $(KEY_EVENT_OBJECTS)
	gcc $(KEY_EVENT_OBJECTS) -o key_event $(LDFLAGS)

cfg_reader: cfg_reader.c config.o
	gcc $(BASIC_CFLAGS) cfg_reader.c -o cfg_reader config.o

.PHONY: clean
clean:
	rm -f *.o list key_event cfg_reader
