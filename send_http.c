#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

#include "send_http.h"

#define MAX_BODY_SIZE 512

struct http_handle
{
    CURL *curl;
    struct curl_slist *headers;
    char body[MAX_BODY_SIZE];
};

static const char* const content_type = 
"Content-Type: application/vnd.com.github.aytchell.eventvalue-v1+json";

static const char* const body_template = "{ \"payload\" : \"%s\" }\r\n";

static size_t dev_null(
        __attribute__((unused)) char *ptr,
        size_t size,
        size_t nmemb,
        __attribute__((unused)) void *userdata)
{
    // ignore the given response
    return size * nmemb;
}

struct http_handle* http_handle_new(const struct config *config)
{
    struct http_handle *handle = (struct http_handle*)malloc(
            sizeof(struct http_handle));

    if (handle == NULL)
    {
        fprintf(stderr, "Failed to allocate http_handle\n");
        http_handle_delete(handle);
        return NULL;
    }

    handle->headers = NULL;
    handle->curl = curl_easy_init();
    if (handle->curl == NULL)
    {
        fprintf(stderr, "Failed to init curl handle\n");
        http_handle_delete(handle);
        return NULL;
    }

    handle->headers = curl_slist_append(handle->headers, content_type);
    curl_easy_setopt(handle->curl, CURLOPT_HTTPHEADER, handle->headers);
    curl_easy_setopt(handle->curl, CURLOPT_URL, config->http_target_url);
    curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, dev_null);

    // CURLOPT_POSTFIELDS and CURLOPT_POSTFIELDSIZE will be set when
    // we're doing the actual call (as the payload and its size is yet
    // unknown)

    return handle;
}

void http_handle_delete(struct http_handle *handle)
{
    if (handle)
    {
        if (handle->headers)
        {
            curl_slist_free_all(handle->headers);
            handle->headers = NULL;
        }
        if (handle->curl)
        {
            curl_easy_cleanup(handle->curl);
            handle->curl = NULL;
        }
        free(handle);
    }
}

static long write_http_body(struct http_handle *handle,
        const struct input_buffer *buffer)
{
    return snprintf(handle->body, MAX_BODY_SIZE, body_template, buffer->text);
}

void send_http_event(struct http_handle *handle,
        const struct input_buffer *buffer)
{
    const long size = write_http_body(handle, buffer);

    curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDSIZE, size);
    curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, handle->body);

    curl_easy_perform(handle->curl);
}
