#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <libevdev/libevdev.h>

#include "input_device.h"

static const char* const input_dir = "/dev/input/";

static int check_input_file(struct input_device *dev)
{
    const int fd = open(dev->filename, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        const int result = -errno;
        fprintf(stderr, "Failed to open '%s': %s\n", dev->filename,
                strerror(errno));
        return result;
    }

    const int rc = libevdev_new_from_fd(fd, &dev->evdev);
    if (rc < 0)
    {
        const int result = -errno;
        fprintf(stderr, "Failed to init libevdev on '%s': %s\n",
                dev->filename, strerror(errno));
        close(fd);
        dev->evdev = NULL;
        return result;
    }

    if ((libevdev_get_id_product(dev->evdev) != dev->product_id) ||
            (libevdev_get_id_vendor(dev->evdev) != dev->vendor_id))
    {
        libevdev_free(dev->evdev);
        dev->evdev = NULL;
        close(fd);
        return 1;
    }

    return 0;
}

static int iterate_dir(DIR *dir, struct input_device *dev)
{
    struct dirent *entry;

    strncpy(dev->filename, input_dir, NAME_MAX - 1);
    const int space_left = NAME_MAX - strlen(input_dir); 
    char* name_loc = dev->filename + strlen(input_dir);

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_CHR)
        {
            if (strncmp(entry->d_name, "event", 5) == 0)
            {
                strncpy(name_loc, entry->d_name, space_left);
                if (0 == check_input_file(dev))
                {
                    // found the requested device
                    return 0;
                }
            }
        }
    }

    return 1;
}

void init_device_struct(struct input_device *dev,
        int vendor_id, int product_id)
{
    dev->vendor_id = vendor_id;
    dev->product_id = product_id;;
    dev->filename[0] = '\0';
    dev->evdev = NULL;
}

int open_input_device(struct input_device *dev)
{
    DIR *dir = opendir(input_dir);
    if (NULL == dir)
    {
        fprintf(stderr, "Failed to open '%s': %s\n", input_dir,
                strerror(errno));
        return -errno;
    }

    const int result = iterate_dir(dir, dev);
    if (0 == result)
    {
        fprintf(stdout, "Detected Barcode-Scanner\n");
    }

    closedir(dir);
    return result;
}

int grab_input_device(struct input_device *dev)
{
    int rc = open_input_device(dev);
    if (0 == rc)
    {
        fprintf(stdout, "Grabbing device\n");
        rc = libevdev_grab(dev->evdev, LIBEVDEV_GRAB);
        if (rc != 0)
        {
            fprintf(stderr, "Failed to grab device\n");
            close_input_device(dev);
            return rc;
        }
    }
    return rc;
}

void close_input_device(struct input_device *dev)
{
    struct libevdev *evdev = dev->evdev;

    if (evdev != NULL)
    {
        const int fd = libevdev_get_fd(evdev);

        libevdev_grab(evdev, LIBEVDEV_UNGRAB);
        libevdev_free(evdev);
        close(fd);

        dev->evdev = NULL;
        dev->filename[0] = '\0';
    }
}

void print_device_info(struct input_device *dev)
{
    if (dev == NULL || dev->evdev == NULL) return;

    struct libevdev *evdev = dev->evdev;

    printf("Event info for '%s'\n", dev->filename);
    printf("Input device name '%s'\n", libevdev_get_name(evdev));
    printf("Physical location '%s'\n", libevdev_get_phys(evdev));
    printf("Unique identifier '%s'\n", libevdev_get_uniq(evdev));
    printf("Product ID '%i'\n", libevdev_get_id_product(evdev));
    printf("Vendor ID '%i'\n", libevdev_get_id_vendor(evdev));
    printf("Bustype ID '%i'\n", libevdev_get_id_bustype(evdev));
    printf("Version ID '%i'\n", libevdev_get_id_version(evdev));
    printf("Driver version '%i'\n", libevdev_get_driver_version(evdev));
}
