#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "logger.h"
#include "handle_scanner.h"
#include "input_device.h"
#include "send_http.h"

int drop_priviledges(const struct config *config)
{
    if (getuid() != 0)
    {
        logger_log(LOG_INFO, "Already running as non-root");
        return TRUE;
    }

    logger_log(LOG_INFO, "Dropping root-priviledges");

    // it's important to first change group id - then user id
    if (setgid(config->nonpriv_gid) != 0)
    {
        logger_log(LOG_ERR, "Failed to change gid: %s", strerror(errno));
        return FALSE;
    }
    if (setuid(config->nonpriv_uid) != 0)
    {
        logger_log(LOG_ERR, "Failed to change uid: %s", strerror(errno));
        return FALSE;
    }

    return TRUE;
}

int grab_scanner_and_scan(const struct config *config)
{
    struct input_device input;
    init_device_struct(&input,
            config->scanner_vendor_id,
            config->scanner_product_id);

    const int rc = grab_input_device(&input);
    if (0 == rc)
    {
        if (!drop_priviledges(config))
        {
            close_input_device(&input);
            return -1;
        }

        if (config->daemonize)
        {
            // daemonize the process.
            // Change cwd to '/' and redirect stdin / stdout / stderr
            if (0 != daemon(0, 0))
            {
                logger_log(LOG_ERR, "Failed to daemonize process: %s",
                        strerror(errno));
                return -1;
            }
            logger_log(LOG_NOTICE, "Now running as daemon in the background");
        }

        struct http_handle *http = http_handle_new(config);
        if (http == NULL)
        {
            logger_log(LOG_CRIT, "Failed to create HTTP handle");
            close_input_device(&input);
            return -1;
        }

        // print_device_info(&input);
        poll_device(input.evdev, config, http);
        close_input_device(&input);
    }

    return rc;
}

int init_config_and_logger(struct config *config)
{
    struct config_logger *cfg_logger = cfg_logger_new();

    set_defaults(config);

    int rc = TRUE;
    rc = read_config(config, "/etc/barcodes.conf", cfg_logger);
    rc = read_config(config, "~/.barcodes.conf", cfg_logger) && rc;

    logger_init(config);
    cfg_logger_flush(cfg_logger);
    cfg_logger_delete(cfg_logger);

    return rc;
}

int main()
{
    struct config config;

    if (!init_config_and_logger(&config))
        return 1;

    int rc = grab_scanner_and_scan(&config);

    logger_log(LOG_NOTICE, "Exiting");
    logger_deinit();

    return rc;
}
