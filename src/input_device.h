#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include <limits.h>

struct libevdev;

struct input_device
{
    int vendor_id;
    int product_id;
    struct libevdev *evdev;
    char filename[NAME_MAX];
};

void init_device_struct(struct input_device *dev,
        int vendor_id, int product_id);

int open_input_device(struct input_device *dev);

int grab_input_device(struct input_device *dev);

void close_input_device(struct input_device *dev);

void print_device_info(struct input_device *dev);

#endif // FILE_SCANNER_H
