#define _GNU_SOURCE 1

#include <libevdev/libevdev.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "input_buffer.h"
#include "input_device.h"

// I didn't find named constants in the headers of libevdev or linux
// so I defined my own based on experimentation
#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define KEY_CONTINUED 2

#define SCANNER_VENDOR_ID   0x28e9
#define SCANNER_PRODUCT_ID  0x03d9

void print_input(struct input_buffer *buffer)
{
    fprintf(stdout, "Read input '%s'\n", buffer->text);
    buffer_clear(buffer);
}

char decode_input(int key_code)
{
    switch (key_code)
    {
        case KEY_0: return '0';
        case KEY_1: return '1';
        case KEY_2: return '2';
        case KEY_3: return '3';
        case KEY_4: return '4';
        case KEY_5: return '5';
        case KEY_6: return '6';
        case KEY_7: return '7';
        case KEY_8: return '8';
        case KEY_9: return '9';
    }

    return '?';
}

void handle_key_released(struct input_event *ev, struct input_buffer *buffer)
{
    switch (ev->code)
    {
        case KEY_ENTER:
            print_input(buffer);
            break;

        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
            buffer_append(buffer, decode_input(ev->code));
            break;

        default:
            printf("Ignoring: %s %s %d\n",
                    libevdev_event_type_get_name(ev->type),
                    libevdev_event_code_get_name(ev->type, ev->code),
                    ev->value);
    }
}

int handle_device_event(struct libevdev *dev, struct input_buffer *buffer)
{
    int rc = 0;
    struct input_event ev;

    do {
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0)
        {
            if ((ev.type == EV_KEY) && (ev.value == KEY_RELEASED))
            {
                handle_key_released(&ev, buffer);
            }
        }
    } while (rc == 1 || rc == 0);
    return 0;
}

int read_device_event(int revents, struct libevdev *dev,
        struct input_buffer *buffer)
{
    if (revents & POLLIN)
    {
        return handle_device_event(dev, buffer);
    }

    switch (revents & (POLLERR | POLLRDHUP))
    {
        case 0:
            printf("fds[0].revents = %i\n", revents);
            break;
        case POLLERR:
            printf("Got 'POLLERR' from input device\n");
            break;
        case POLLRDHUP:
            printf("Got 'POLLRDHUP' from input device\n");
            break;
        case POLLERR | POLLRDHUP:
            printf("Got 'POLLERR | POLLRDHUP' from input device\n");
            break;
    }

    return -1;
}

void poll_device(struct libevdev *dev)
{
    int rc = 0;
    struct pollfd fds[1];
    int timeout = 5000;
    struct input_buffer buffer;

    if (!buffer_new(512, &buffer))
    {
        fprintf(stderr, "Failed to allocate input buffer\n");
        return;
    }

    fds[0].fd = libevdev_get_fd(dev);
    fds[0].events = POLLIN | POLLRDHUP | POLLERR;
    fds[0].revents = 0;

    for (;;)
    {
        rc = poll(fds, 1, timeout);
        if (rc > 0)
        {
            if (0 != read_device_event(fds[0].revents, dev, &buffer))
            {
                return;
            }
        }
        else
        {
            printf("poll returned %i\n", rc);
            return;
        }
    }
}

int main(int argc, char* argv[]) {
    struct input_device input;
    init_device_struct(&input, SCANNER_VENDOR_ID, SCANNER_PRODUCT_ID);

    const int rc = grab_input_device(&input);
    if (0 == rc)
    {
        // print_device_info(&input);
        poll_device(input.evdev);
        close_input_device(&input);
    }

    return rc;
}
