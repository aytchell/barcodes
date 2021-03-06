#define _GNU_SOURCE

#include <libevdev/libevdev.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>

#include "logger.h"
#include "input_buffer.h"
#include "send_http.h"
#include "handle_scanner.h"

// I didn't find named constants in the headers of libevdev or linux
// so I defined my own based on experimentation
#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define KEY_CONTINUED 2

static void process_read_buffer(struct input_buffer *buffer,
        struct http_handle *http)
{
    logger_log(LOG_INFO, "Read barcode: %s", buffer->text);
    send_http_event(http, buffer);
    buffer_clear(buffer);
}

static char decode_input(int key_code)
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

static void handle_key_released(struct input_event *ev,
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
            logger_log(LOG_DEBUG, "Ignoring: %s %s %d",
                    libevdev_event_type_get_name(ev->type),
                    libevdev_event_code_get_name(ev->type, ev->code),
                    ev->value);
    }
}

static int handle_device_event(struct libevdev *dev,
        struct input_buffer *buffer, struct http_handle *http)
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

static int read_device_event(int revents, struct libevdev *dev,
        struct input_buffer *buffer, struct http_handle *http)
{
    if (revents & POLLIN)
    {
        return handle_device_event(dev, buffer, http);
    }

    switch (revents & (POLLERR | POLLRDHUP))
    {
        case 0:
            logger_log(LOG_INFO, "fds[0].revents = %i", revents);
            break;
        case POLLERR:
            logger_log(LOG_INFO, "Got 'POLLERR' from input device");
            break;
        case POLLRDHUP:
            logger_log(LOG_INFO, "Got 'POLLRDHUP' from input device");
            break;
        case POLLERR | POLLRDHUP:
            logger_log(LOG_INFO, "Got 'POLLERR | POLLRDHUP' from input device");
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
        logger_log(LOG_CRIT, "Failed to allocate input buffer");
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
            logger_log(LOG_ERR, "poll returned %i", rc);
            return;
        }
    }
}
