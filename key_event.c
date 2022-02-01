#include <libevdev/libevdev.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    struct libevdev *dev = NULL;
    int fd;
    int rc = 1;
    const char* device = "/dev/input/event0";

    if (argc > 1)
    {
        device = argv[1];
    }

    fd = open(device, O_RDONLY | O_NONBLOCK);
    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
        fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
        exit(1);
    }

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
