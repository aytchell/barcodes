#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libevdev/libevdev.h>

#include "input_device.h"
#include "logger.h"

static const char* const input_dir = "/dev/input/";

static void print_device_info(struct input_device *dev)
{
    if (dev == NULL || dev->evdev == NULL) return;

    struct libevdev *evdev = dev->evdev;

    logger_log(LOG_DEBUG, "Event info for '%s'", dev->filename);
    logger_log(LOG_DEBUG, "Input device name '%s'", libevdev_get_name(evdev));
    logger_log(LOG_DEBUG, "Physical location '%s'", libevdev_get_phys(evdev));
    logger_log(LOG_DEBUG, "Unique identifier '%s'", libevdev_get_uniq(evdev));
    logger_log(LOG_DEBUG, "Product ID 0x%x", libevdev_get_id_product(evdev));
    logger_log(LOG_DEBUG, "Vendor ID 0x%x", libevdev_get_id_vendor(evdev));
    logger_log(LOG_DEBUG, "Bustype ID %i", libevdev_get_id_bustype(evdev));
    logger_log(LOG_DEBUG, "Version ID %i", libevdev_get_id_version(evdev));
    logger_log(LOG_DEBUG, "Driver version %i", libevdev_get_driver_version(evdev));
}

static int check_input_file(struct input_device *dev)
{
    const int fd = open(dev->filename, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        const int result = -errno;
        logger_log(LOG_WARNING, "Failed to open '%s': %s", dev->filename,
                strerror(errno));
        return result;
    }

    const int rc = libevdev_new_from_fd(fd, &dev->evdev);
    if (rc < 0)
    {
        const int result = -errno;
        logger_log(LOG_ERR, "Failed to init libevdev on '%s': %s",
                dev->filename, strerror(errno));
        close(fd);
        dev->evdev = NULL;
        return result;
    }

    if ((libevdev_get_id_product(dev->evdev) != dev->product_id) ||
            (libevdev_get_id_vendor(dev->evdev) != dev->vendor_id))
    {
        logger_log(LOG_DEBUG, "Input device '%s' is no scanner",
                dev->filename);
        libevdev_free(dev->evdev);
        dev->evdev = NULL;
        close(fd);
        return 1;
    }

    logger_log(LOG_NOTICE, "Detected barcode scanner at '%s'",
            dev->filename);
    print_device_info(dev);
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

    logger_log(LOG_NOTICE, "Barcode scanner not found");
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
    logger_log(LOG_DEBUG, "Scanning '%s' for barcode scanner", input_dir);
    logger_log(LOG_INFO, "Expectation: vendor(0x%x), product(0x%x)",
            dev->vendor_id, dev->product_id);

    DIR *dir = opendir(input_dir);
    if (NULL == dir)
    {
        logger_log(LOG_WARNING, "Failed to open '%s': %s", input_dir,
                strerror(errno));
        return -errno;
    }

    const int result = iterate_dir(dir, dev);
    closedir(dir);
    return result;
}

int grab_input_device(struct input_device *dev)
{
    int rc = open_input_device(dev);
    if (0 == rc)
    {
        logger_log(LOG_INFO, "Grabbing device");
        rc = libevdev_grab(dev->evdev, LIBEVDEV_GRAB);
        if (rc != 0)
        {
            logger_log(LOG_ERR, "Failed to grab device");
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
