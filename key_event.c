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

void poll_device(int fd, struct libevdev *dev)
{
    int rc = 0;
    struct pollfd fds[1];
    int timeout = 5000;
    struct input_event ev;

    fds[0].fd = fd;
    fds[0].events = POLLIN | POLLPRI | POLLRDHUP | POLLERR;

    for (;;)
    {
        fds[0].revents = 0;
        rc = poll(fds, 1, timeout);
        printf("poll returned %i\n", rc);
        if (rc > 0)
        {
            printf("fds[0].revents = %i\n", fds[0].revents);
            printf("POLLIN = %i\n", POLLIN);
            printf("POLLPRI = %i\n", POLLPRI);
            printf("POLLOUT = %i\n", POLLOUT);
            printf("POLLRDHUP = %i\n", POLLRDHUP);
            printf("POLLERR = %i\n", POLLERR);
            printf("POLLHUP = %i\n", POLLHUP);
            printf("POLLNVAL = %i\n", POLLNVAL);

            if (fds[0].revents & POLLIN)
            {
                do {
                    rc = libevdev_next_event(dev,
                            LIBEVDEV_READ_FLAG_NORMAL, &ev);
                    if (rc == 0)
                    {
                        printf("Event: %s %s %d\n",
                                libevdev_event_type_get_name(ev.type),
                                libevdev_event_code_get_name(ev.type, ev.code),
                                ev.value);
                    }
                } while (rc == 1 || rc == 0);
            }
        }
        else
        {
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

int main(int argc, char* argv[]) {
    struct libevdev *dev = NULL;
    int fd = -1;
    int rc = 1;
    const char* device = "/dev/input/event0";

    if (argc > 1)
    {
        device = argv[1];
    }

    fd = open(device, O_RDONLY | O_NONBLOCK);
    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0 || fd < 0) {
        fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
        exit(1);
    }

    print_device_info(device, dev);
    poll_device(fd, dev);
    close(fd);
}
