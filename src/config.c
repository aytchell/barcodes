#include "config.h"
#include "logger.h"
#include <string.h>

#define SCANNER_VENDOR_ID   0x28e9
#define SCANNER_PRODUCT_ID  0x03d9

#define NONPRIV_GROUP_ID    1000
#define NONPRIV_USER_ID     1000

#define NEVER_TIMEOUT       -1
#define JAMBEL_EVENT_URL    "http://localhost:8080/event-input/558a4918-54a7-492c-b268-3790f4d5f0f5"
#define HTTP_CONTENT_TYPE   "application/vnd.com.github.aytchell.eventvalue-v1+json"

static void set_defaults(struct config *config)
{
    config->scanner_vendor_id = SCANNER_VENDOR_ID;
    config->scanner_product_id = SCANNER_PRODUCT_ID;
    config->nonpriv_gid = NONPRIV_GROUP_ID;
    config->nonpriv_uid = NONPRIV_USER_ID;
    config->scan_timeout = NEVER_TIMEOUT;
    config->log_threshold = LOG_DEBUG;
    strncpy(config->http_upload_verb, "PUT", MAX_UPLOAD_VERB_LEN);
    strncpy(config->http_target_url, JAMBEL_EVENT_URL, MAX_TARGET_URL_LEN);
    strncpy(config->http_content_type, JAMBEL_EVENT_URL, MAX_CONTENT_TYPE_LEN);
    strncpy(config->http_json_payload_name, "payload", MAX_PAYLOAD_NAME_LEN);
}

static int read_file(
        __attribute__((unused)) struct config *config,
        __attribute__((unused)) const char *filename)
{
    // to be implemented
    return TRUE;
}

int read_config(struct config *config)
{
    set_defaults(config);
    return read_file(config, "/etc/barcodes.config");
}
