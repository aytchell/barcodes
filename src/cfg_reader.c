#include <stdio.h>
#include "config.h"

int main()
{
    struct config config;

    if (!read_config(&config))
    {
        fprintf(stderr, "Failed to read config\n");
        return 1;
    }

    fprintf(stdout, "scanner_vendor_id = %i\n", config.scanner_vendor_id);
    fprintf(stdout, "scanner_product_id = %i\n", config.scanner_product_id);
    fprintf(stdout, "nonpriv_gid = %i\n", config.nonpriv_gid);
    fprintf(stdout, "nonpriv_uid = %i\n", config.nonpriv_uid);
    fprintf(stdout, "scan_timeout = %i\n", config.scan_timeout);
    fprintf(stdout, "http_upload_verb = '%s'\n", config.http_upload_verb);
    fprintf(stdout, "http_target_url = '%s'\n", config.http_target_url);
    fprintf(stdout, "http_content_type = '%s'\n", config.http_content_type);
    fprintf(stdout, "http_json_payload_name = '%s'\n", config.http_json_payload_name);
}
