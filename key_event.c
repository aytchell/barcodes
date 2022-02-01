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

// I didn't find named constants in the headers of libevdev or linux
// so I defined my own based on experimentation
#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define KEY_CONTINUED 2

int handle_device_event(struct libevdev *dev)
{
    int rc = 0;
    struct input_event ev;

    do {
        rc = libevdev_next_event(dev,
                LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0)
        {
            if ((ev.type == EV_KEY) && (ev.value == KEY_RELEASED))
            {
                printf("Event: %s %s %d\n",
                        libevdev_event_type_get_name(ev.type),
                        libevdev_event_code_get_name(ev.type, ev.code),
                        ev.value);
            }
        }
    } while (rc == 1 || rc == 0);
    return 0;
}

int read_device_event(int revents, struct libevdev *dev)
{
    if (revents & POLLIN)
    {
        return handle_device_event(dev);
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

    fds[0].fd = libevdev_get_fd(dev);
    fds[0].events = POLLIN | POLLRDHUP | POLLERR;
    fds[0].revents = 0;

    for (;;)
    {
        rc = poll(fds, 1, timeout);
        if (rc > 0)
        {
            if (0 != read_device_event(fds[0].revents, dev))
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

void print_device_info(const char *device, struct libevdev *dev)
{
    printf("Event info for '%s'\n", device);
    printf("Input device name '%s'\n", libevdev_get_name(dev));
    printf("Physical location '%s'\n", libevdev_get_phys(dev));
    printf("Unique identifier '%s'\n", libevdev_get_uniq(dev));
    printf("Product ID '%i'\n", libevdev_get_id_product(dev));
    printf("Vendor ID '%i'\n", libevdev_get_id_vendor(dev));
    printf("Bustype ID '%i'\n", libevdev_get_id_bustype(dev));
    printf("Version ID '%i'\n", libevdev_get_id_version(dev));
    printf("Driver version '%i'\n", libevdev_get_driver_version(dev));
}

struct libevdev* init_device(const char* device_name)
{
    struct libevdev* dev = NULL;

    const int fd = open(device_name, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open '%s'\n", device_name);
        return NULL;
    }

    const int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0)
    {
        close(fd);
        fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
        return NULL;
    }

    return dev;
}

void deinit_device(struct libevdev *dev)
{
    const int fd = libevdev_get_fd(dev);

    libevdev_free(dev);
    close(fd);
}

const char* get_device_name(int argc, char *argv[])
{
    if (argc > 1)
    {
        return argv[1];
    }
    else
    {
        return "/dev/input/event0";
    }
}

int main(int argc, char* argv[]) {
    const char* device = get_device_name(argc, argv);
    struct libevdev *dev = init_device(device);

    if (dev != NULL)
    {
        print_device_info(device, dev);
        poll_device(dev);
        deinit_device(dev);
    }
}
