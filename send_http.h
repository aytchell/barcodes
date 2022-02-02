#ifndef SEND_HTTP_H
#define SEND_HTTP_H

#include "input_buffer.h"
#include "config.h"

struct http_handle;

struct http_handle* http_handle_new(const struct config *config);

void http_handle_delete(struct http_handle *handle);

void send_http_event(struct http_handle *handle,
        const struct input_buffer *buffer);

#endif // SEND_HTTP_H
