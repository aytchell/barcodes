#ifndef HANDLE_SCANNER_H
#define HANDLE_SCANNER_H

struct libevdev;
struct config;
struct http_handle;

void poll_device(struct libevdev *dev, const struct config *config,
        struct http_handle *http);

#endif // HANDLE_SCANNER_H
