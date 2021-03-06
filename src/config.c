#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "config.h"
#include "logger.h"
#include "string_utils.h"

#define SCANNER_VENDOR_ID   -1
#define SCANNER_PRODUCT_ID  -1

#define NONPRIV_GROUP       "nobody"
#define NONPRIV_USER        "nogroup"

#define NEVER_TIMEOUT       -1
#define HTTP_TARGET_URL     "http://localhost:8080/barcode"
#define HTTP_CONTENT_TYPE   "application/json"

#define MAX_CONFIG_LINE_LEN 768

struct context
{
    const char *filename;
    int line_nr;
    struct config_logger *logger;
    struct config *config;
};

static int process_uid(const char *value, struct context *context);
static int process_gid(const char *value, struct context *context);

void set_defaults(struct config *config)
{
    config->scanner_vendor_id = SCANNER_VENDOR_ID;
    config->scanner_product_id = SCANNER_PRODUCT_ID;
    config->scan_timeout = NEVER_TIMEOUT;
    config->log_threshold = LOG_DEBUG;
    config->log_to_syslog = FALSE;
    config->daemonize = FALSE;
    strncpy(config->http_upload_verb, "POST", MAX_UPLOAD_VERB_LEN);
    strncpy(config->http_target_url, HTTP_TARGET_URL, MAX_TARGET_URL_LEN);
    strncpy(config->http_content_type, HTTP_CONTENT_TYPE, MAX_CONTENT_TYPE_LEN);
    strncpy(config->http_json_payload_name, "payload", MAX_PAYLOAD_NAME_LEN);

    struct context context;
    context.filename = NULL;
    context.line_nr = 0;
    context.logger = NULL;
    context.config = config;

    process_uid(NONPRIV_USER, &context);
    process_gid(NONPRIV_GROUP, &context);
}

static int process_opt_int_entry(const char *value, struct context *context,
        const char *name, int *storage, int is_mandatory)
{
    const int int_val = parse_uint16(value);
    if (int_val == -1)
    {
        if (is_mandatory)
        {
            cfg_logger_log(context->logger, LOG_ERR,
                    "Invalid entry (%s) for '%s' in %s:%i",
                    value, name, context->filename, context->line_nr);
        }
        return FALSE;
    }

    cfg_logger_log(context->logger, LOG_DEBUG,
            "Found %i for '%s' in %s:%i",
            int_val, name, context->filename, context->line_nr);
    *storage = int_val;
    return TRUE;
}

static int process_int_entry(const char *value, struct context *context,
        const char *name, int *storage)
{
    return process_opt_int_entry(value, context, name, storage, TRUE);
}

static int process_vendor_id(const char *value, struct context *context)
{
    return process_int_entry(value, context, "vendor_id",
            &(context->config->scanner_vendor_id));
}

static int process_product_id(const char *value, struct context *context)
{
    return process_int_entry(value, context, "product_id",
            &(context->config->scanner_product_id));
}

static int process_uid(const char *value, struct context *context)
{
    if (process_opt_int_entry(value, context, "uid",
            &(context->config->nonpriv_uid), FALSE))
    {
        // Found integer value which is used as user id
        return TRUE;
    }

    struct passwd *pwd = getpwnam(value);
    if (pwd != NULL)
    {
        cfg_logger_log(context->logger, LOG_DEBUG,
                "Found '%s' (uid %i) for 'uid' in %s:%i",
                value, pwd->pw_uid, context->filename, context->line_nr);
        context->config->nonpriv_uid = pwd->pw_uid;
        return TRUE;
    }

    cfg_logger_log(context->logger, LOG_ERR,
            "Invalid entry (%s) for 'uid' in %s:%i",
            value, context->filename, context->line_nr);
    return FALSE;
}

static int process_gid(const char *value, struct context *context)
{
    if (process_opt_int_entry(value, context, "gid",
            &(context->config->nonpriv_gid), FALSE))
    {
        // Found integer value which is used as group id
        return TRUE;
    }

    struct group *grp = getgrnam(value);
    if (grp != NULL)
    {
        cfg_logger_log(context->logger, LOG_DEBUG,
                "Found '%s' (gid %i) for 'gid' in %s:%i",
                value, grp->gr_gid, context->filename, context->line_nr);
        context->config->nonpriv_uid = grp->gr_gid;
        return TRUE;
    }

    cfg_logger_log(context->logger, LOG_ERR,
            "Invalid entry (%s) for 'gid' in %s:%i",
            value, context->filename, context->line_nr);
    return FALSE;
}

static int parse_log_level(const char *value)
{
    if (0 == strcasecmp("CRITICAL", value)) return LOG_CRIT;
    if (0 == strcasecmp("ERROR", value))    return LOG_ERR;
    if (0 == strcasecmp("WARNING", value))  return LOG_WARNING;
    if (0 == strcasecmp("NOTICE", value))   return LOG_NOTICE;
    if (0 == strcasecmp("INFO", value))     return LOG_INFO;
    if (0 == strcasecmp("DEBUG", value))    return LOG_DEBUG;

    return -1;
}

static int process_log_threshold(const char *value, struct context *context)
{
    struct config *config = context->config;

    const int val = parse_log_level(value);
    if (val == -1)
    {
        cfg_logger_log(context->logger, LOG_ERR,
                "Invalid log level '%s' in %s:%i", value,
                context->filename, context->line_nr);
        return FALSE;
    }

    config->log_threshold = val;
    return TRUE;
}

static int process_boolean_param(const char *value, struct context *context,
        const char *name,
        const char *log_on_true, const char *log_on_false, int *storage)
{
    if (
            (0 == strcasecmp("TRUE", value)) ||
            (0 == strcasecmp("YES", value)))
    {
        cfg_logger_log(context->logger, LOG_DEBUG, "%s (%s:%i)", log_on_true,
                context->filename, context->line_nr);
        *storage = TRUE;
        return TRUE;
    }

    if (
            (0 == strcasecmp("FALSE", value)) ||
            (0 == strcasecmp("NO", value)))
    {
        cfg_logger_log(context->logger, LOG_DEBUG, "%s (%s:%i)", log_on_false,
                context->filename, context->line_nr);
        *storage = FALSE;
        return TRUE;
    }

    cfg_logger_log(context->logger, LOG_ERR,
            "Invalid entry '%s' for %s in %s:%i", value, name,
            context->filename, context->line_nr);
    return FALSE;
}

static int process_use_syslog(const char *value, struct context *context)
{
    return process_boolean_param(value, context, "use_syslog",
            "Using syslog", "Using stdout/stderr",
            &(context->config->log_to_syslog));
}

static int process_daemonize(const char *value, struct context *context)
{
    return process_boolean_param(value, context, "daemonize",
            "Will daemonize process", "Will stay in foreground",
            &(context->config->daemonize));
}

static int process_http_upload_verb(const char *value, struct context *context)
{
    if (
            (0 == strcmp("PUT", value)) ||
            (0 == strcmp("POST", value)) ||
            (0 == strcmp("PATCH", value)))
    {
        cfg_logger_log(context->logger, LOG_DEBUG,
                "Using '%s' for HTTP requests (%s:%i)", value,
                context->filename, context->line_nr);
        struct config *config = context->config;
        strncpy(config->http_upload_verb, value, MAX_UPLOAD_VERB_LEN);
        return TRUE;
    }

    cfg_logger_log(context->logger, LOG_ERR,
            "Invalid entry '%s' for http_upload_verb in %s:%i", value,
            context->filename, context->line_nr);
    return FALSE;
}

static int process_http_target_url(const char *value, struct context *context)
{
    struct config *config = context->config;
    strncpy(config->http_target_url, value, MAX_TARGET_URL_LEN);
    cfg_logger_log(context->logger, LOG_DEBUG,
            "Using '%s' for http_target_url (%s:%i)",
            config->http_target_url, context->filename, context->line_nr);
    return TRUE;
}

static int process_http_content_type(const char *value, struct context *context)
{
    struct config *config = context->config;
    strncpy(config->http_content_type, value, MAX_CONTENT_TYPE_LEN);
    cfg_logger_log(context->logger, LOG_DEBUG,
            "Using '%s' for http_content_type (%s:%i)",
            config->http_content_type, context->filename, context->line_nr);
    return TRUE;
}

static int process_http_json_payload_name(const char *value,
        struct context *context)
{
    struct config *config = context->config;
    strncpy(config->http_json_payload_name, value, MAX_PAYLOAD_NAME_LEN);
    cfg_logger_log(context->logger, LOG_DEBUG,
            "Using '%s' for http_json_payload_name (%s:%i)",
            config->http_json_payload_name,
            context->filename, context->line_nr);
    return TRUE;
}

static int process_param(const char* key, const char *value,
        struct context *context)
{
    if (0 == strcmp("vendor_id", key))
        return process_vendor_id(value, context);

    if (0 == strcmp("product_id", key))
        return process_product_id(value, context);

    if (0 == strcmp("uid", key))
        return process_uid(value, context);

    if (0 == strcmp("gid", key))
        return process_gid(value, context);

    if (0 == strcmp("log_threshold", key))
        return process_log_threshold(value, context);

    if (0 == strcmp("use_syslog", key))
        return process_use_syslog(value, context);

    if (0 == strcmp("daemonize", key))
        return process_daemonize(value, context);

    if (0 == strcmp("http_upload_verb", key))
        return process_http_upload_verb(value, context);

    if (0 == strcmp("http_target_url", key))
        return process_http_target_url(value, context);

    if (0 == strcmp("http_content_type", key))
        return process_http_content_type(value, context);

    if (0 == strcmp("http_json_payload_name", key))
        return process_http_json_payload_name(value, context);

    cfg_logger_log(context->logger, LOG_ERR,
            "Encountered unknown keyword '%s' in %s:%i", key,
            context->filename, context->line_nr);
    return FALSE;
}

static int parse_param_line(char* line, struct context *context)
{
    char* equal = strchr(line, '=');
    if (equal == NULL)
    {
        cfg_logger_log(context->logger, LOG_ERR,
                "No '=' found in config file %s:%i",
                context->filename, context->line_nr);
        return FALSE;
    }

    *equal = '\0';
    const char *key = trim(line);
    const char *value = trim(++equal);

    return process_param(key, value, context);
}

static int parse_raw_line(char* line, struct context *context)
{
    char *comment = strchr(line, '#');
    if (comment != NULL)
    {
        // Strip everything beyond a '#' character (i.e. ignore comments)
        *comment = '\0';
    }

    line = find_first_not_of(line, " \t\r\n");
    if (*line == '\0')
    {
        // empty line (or only spaces or comments)
        return TRUE;
    }

    return parse_param_line(line, context);
}

int read_config_file(FILE *file, struct context *context)
{
    char buffer[MAX_CONFIG_LINE_LEN];
    char* line;
    int result = TRUE;

    context->line_nr = 0;

    while ((line = fgets(buffer, MAX_CONFIG_LINE_LEN, file)) != NULL)
    {
        ++context->line_nr;
        if (!parse_raw_line(line, context))
        {
            // we continue to (hopefully) load the correct logger config
            result = FALSE;
        }
    }

    return result;
}

int read_config(struct config *config, const char *filename,
        struct config_logger *logger)
{
    FILE* file = fopen(filename, "r");

    if (file == NULL)
    {
        cfg_logger_log(logger, LOG_INFO, "Failed to open %s", filename);
        return TRUE;
    }

    struct context context;
    context.filename = filename;
    context.line_nr = 0;
    context.logger = logger;
    context.config = config;

    const int rc = read_config_file(file, &context);
    fclose(file);

    return rc;
}
