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
#include "send_http.h"
#include "config.h"

// I didn't find named constants in the headers of libevdev or linux
// so I defined my own based on experimentation
#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define KEY_CONTINUED 2

#define SCANNER_VENDOR_ID   0x28e9
#define SCANNER_PRODUCT_ID  0x03d9

#define NONPRIV_GROUP_ID    1000
#define NONPRIV_USER_ID     1000

#define NEVER_TIMEOUT       -1
#define JAMBEL_EVENT_URL    "http://localhost:8080/event-input/558a4918-54a7-492c-b268-3790f4d5f0f5"

int drop_priviledges(const struct config *config)
{
    if (getuid() != 0)
    {
        fprintf(stdout, "Already running as non-root\n");
        return TRUE;
    }

    fprintf(stdout, "Dropping root-priviledges\n");

    // it's important to first change group id - then user id
    if (setgid(config->nonpriv_gid) != 0)
    {
        fprintf(stderr, "Failed to change gid: %s\n", strerror(errno));
        return FALSE;
    }
    if (setuid(config->nonpriv_uid) != 0)
    {
        fprintf(stderr, "Failed to change uid: %s\n", strerror(errno));
        return FALSE;
    }

    return TRUE;
}

void process_read_buffer(struct input_buffer *buffer, struct http_handle *http)
{
    send_http_event(http, buffer);
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

void handle_key_released(struct input_event *ev,
        struct input_buffer *buffer, struct http_handle *http)
{
    switch (ev->code)
    {
        case KEY_ENTER:
            process_read_buffer(buffer, http);
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

int handle_device_event(struct libevdev *dev, struct input_buffer *buffer,
        struct http_handle *http)
{
    int rc = 0;
    struct input_event ev;

    do {
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0)
        {
            if ((ev.type == EV_KEY) && (ev.value == KEY_RELEASED))
            {
                handle_key_released(&ev, buffer, http);
            }
        }
    } while (rc == 1 || rc == 0);
    return 0;
}

int read_device_event(int revents, struct libevdev *dev,
        struct input_buffer *buffer, struct http_handle *http)
{
    if (revents & POLLIN)
    {
        return handle_device_event(dev, buffer, http);
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

void poll_device(struct libevdev *dev, const struct config *config,
        struct http_handle *http)
{
    int rc = 0;
    struct pollfd fds[1];
    const int timeout = config->scan_timeout;
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
            if (0 != read_device_event(fds[0].revents, dev, &buffer, http))
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

int grab_scanner_and_scan(const struct config *config)
{
    struct input_device input;
    init_device_struct(&input,
            config->scanner_vendor_id,
            config->scanner_product_id);

    const int rc = grab_input_device(&input);
    if (0 == rc)
    {
        drop_priviledges(config);

        struct http_handle *http = http_handle_new(config);
        if (http == NULL)
        {
            fprintf(stderr, "Failed to create HTTP handle\n");
            close_input_device(&input);
            return -1;
        }

        // print_device_info(&input);
        poll_device(input.evdev, config, http);
        close_input_device(&input);
    }

    return rc;
}

int main()
{
    struct config config;

    config.scanner_vendor_id = SCANNER_VENDOR_ID;
    config.scanner_product_id = SCANNER_PRODUCT_ID;
    config.nonpriv_gid = NONPRIV_GROUP_ID;
    config.nonpriv_uid = NONPRIV_USER_ID;
    config.scan_timeout = NEVER_TIMEOUT;
    strncpy(config.http_target_url, JAMBEL_EVENT_URL, 512);

    return grab_scanner_and_scan(&config);
}
