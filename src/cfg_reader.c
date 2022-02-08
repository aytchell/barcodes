#include <stdio.h>
#include "config.h"
#include "logger.h"

int init_config_and_logger(struct config *config)
{
    struct config_logger *cfg_logger = cfg_logger_new();

    set_defaults(config);
    const int rc = read_config(config, "barcodes.conf", cfg_logger);

    logger_init(config);
    cfg_logger_flush(cfg_logger);
    cfg_logger_delete(cfg_logger);

    return rc;
}

int main()
{
    struct config config;
    set_defaults(&config);

    if (!init_config_and_logger(&config))
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
