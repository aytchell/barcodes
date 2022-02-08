#ifndef CONFIG_H
#define CONFIG_H

#include "cfg_logger.h"

#define TRUE (1 == 1)
#define FALSE (!(TRUE))

#define MAX_UPLOAD_VERB_LEN       8
#define MAX_TARGET_URL_LEN      512
#define MAX_CONTENT_TYPE_LEN    256
#define MAX_PAYLOAD_NAME_LEN     32

struct config
{
    int scanner_vendor_id;
    int scanner_product_id;
    int nonpriv_gid;
    int nonpriv_uid;
    int scan_timeout;
    int log_threshold; // log only if level <= log_threshold
    int log_to_syslog;
    char http_upload_verb[MAX_UPLOAD_VERB_LEN + 1];
    char http_target_url[MAX_TARGET_URL_LEN + 1];
    char http_content_type[MAX_CONTENT_TYPE_LEN + 1];
    char http_json_payload_name[MAX_PAYLOAD_NAME_LEN + 1];
};

void set_defaults(struct config *config);

int read_config(struct config *config, const char* filename,
        struct config_logger *logger);


#endif // CONFIG_H
