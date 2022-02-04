#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#include "send_http.h"
#include "logger.h"

#define MAX_BODY_SIZE 512

struct http_handle
{
    CURL *curl;
    struct curl_slist *headers;
    char json_name[MAX_PAYLOAD_NAME_LEN + 1];
    char body[MAX_BODY_SIZE];
    char errorbuf[CURL_ERROR_SIZE];
};

static const char* const body_template = "{ \"%s\" : \"%s\" }\r\n";

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
        logger_log(LOG_ERR, "Failed to allocate http_handle");
        http_handle_delete(handle);
        return NULL;
    }

    handle->headers = NULL;
    handle->curl = curl_easy_init();
    if (handle->curl == NULL)
    {
        logger_log(LOG_ERR, "Failed to init curl handle");
        http_handle_delete(handle);
        return NULL;
    }

    strncpy(handle->json_name, config->http_json_payload_name,
            MAX_PAYLOAD_NAME_LEN);

    char content_type[MAX_CONTENT_TYPE_LEN + 20];
    snprintf(content_type, MAX_CONTENT_TYPE_LEN + 20, "Content-Type: %s",
            config->http_content_type);
    handle->headers = curl_slist_append(handle->headers, content_type);
    curl_easy_setopt(handle->curl, CURLOPT_HTTPHEADER, handle->headers);
    curl_easy_setopt(handle->curl, CURLOPT_URL, config->http_target_url);
    curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST,
            config->http_upload_verb);
    curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, dev_null);
    curl_easy_setopt(handle->curl, CURLOPT_ERRORBUFFER, handle->errorbuf);
    logger_log(LOG_DEBUG, "Initialized curl handle");
    logger_log(LOG_DEBUG, "Target url is '%s'", config->http_target_url);
    logger_log(LOG_DEBUG, "Used HTTP verb is '%s'", config->http_upload_verb);

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
        logger_log(LOG_INFO, "Disposed curl handle");
    }
}

static long write_http_body(struct http_handle *handle,
        const struct input_buffer *buffer)
{
    return snprintf(handle->body, MAX_BODY_SIZE,
            body_template, handle->json_name, buffer->text);
}

void send_http_event(struct http_handle *handle,
        const struct input_buffer *buffer)
{
    const long size = write_http_body(handle, buffer);

    curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDSIZE, size);
    curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, handle->body);

    logger_log(LOG_NOTICE, "Uploading barcode to REST endpoint");
    logger_log(LOG_DEBUG, "Request body is '%s'", handle->body);

    const CURLcode res = curl_easy_perform(handle->curl);
    if (res == CURLE_OK)
    {
        logger_log(LOG_DEBUG, "Request successfully sent");
    }
    else
    {
        logger_log(LOG_WARNING, "An error occured: %s", handle->errorbuf);
    }
}
