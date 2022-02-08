#include "config.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define SCANNER_VENDOR_ID   -1
#define SCANNER_PRODUCT_ID  -1

#define NONPRIV_GROUP_ID    65534
#define NONPRIV_USER_ID     65534

#define NEVER_TIMEOUT       -1
#define JAMBEL_EVENT_URL    "http://localhost:8080/barcode"
#define HTTP_CONTENT_TYPE   "application/json"

#define MAX_CONFIG_LINE_LEN 768

void set_defaults(struct config *config)
{
    config->scanner_vendor_id = SCANNER_VENDOR_ID;
    config->scanner_product_id = SCANNER_PRODUCT_ID;
    config->nonpriv_uid = NONPRIV_USER_ID;
    config->nonpriv_gid = NONPRIV_GROUP_ID;
    config->scan_timeout = NEVER_TIMEOUT;
    config->log_threshold = LOG_DEBUG;
    config->log_to_syslog = FALSE;
    strncpy(config->http_upload_verb, "POST", MAX_UPLOAD_VERB_LEN);
    strncpy(config->http_target_url, JAMBEL_EVENT_URL, MAX_TARGET_URL_LEN);
    strncpy(config->http_content_type, HTTP_CONTENT_TYPE, MAX_CONTENT_TYPE_LEN);
    strncpy(config->http_json_payload_name, "payload", MAX_PAYLOAD_NAME_LEN);
}

static int matches_any_of(char current, char* patterns)
{
    char *p = patterns;
    while (*p != '\0')
    {
        if (*p == current)
        {
            return TRUE;
        }
        ++p;
    }

    return FALSE;
}

static char* find_first_not_of(char *line, char *patterns)
{
    while (*line != '\0')
    {
        char c = line[0];
        if (!matches_any_of(c, patterns))
        {
            return line;
        }
        ++line;
    }

    return line;
}

static char* find_last_not_of(char *line, char *patterns)
{
    char *last = line;
    char *pos = line;

    while (*pos != '\0')
    {
        if (!matches_any_of(*pos, patterns))
        {
            last = pos;
        }
        ++pos;
    }

    return last;
}

static int parse_int(const char *value)
{
    char *endptr = NULL;
    int base = 10;
    if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
    {
        base = 16;
    }

    long val = strtol(value, &endptr, base);
    if (*endptr != '\0') return FALSE;
    if (val > SHRT_MAX) return FALSE;
    if (val < 0) return FALSE;
    return val;
}

static int process_vendor_id(const char *value, struct config *config)
{
    const int int_val = parse_int(value);
    if (int_val == -1)
        return FALSE;
    config->scanner_vendor_id = int_val;
    return TRUE;
}

static int process_product_id(const char *value, struct config *config)
{
    const int int_val = parse_int(value);
    if (int_val == -1)
        return FALSE;
    config->scanner_product_id = int_val;
    return TRUE;
}

static int process_uid(const char *value, struct config *config)
{
    const int int_val = parse_int(value);
    if (int_val == -1)
        return FALSE;
    config->nonpriv_uid = int_val;
    return TRUE;
}

static int process_gid(const char *value, struct config *config)
{
    const int int_val = parse_int(value);
    if (int_val == -1)
        return FALSE;
    config->nonpriv_gid = int_val;
    return TRUE;
}

static int process_log_threshold(const char *value, struct config *config)
{
    if (0 == strcasecmp("CRITICAL", value))
    {
        config->log_threshold = LOG_CRIT;
        return TRUE;
    }

    if (0 == strcasecmp("ERROR", value))
    {
        config->log_threshold = LOG_ERR;
        return TRUE;
    }

    if (0 == strcasecmp("WARNING", value))
    {
        config->log_threshold = LOG_WARNING;
        return TRUE;
    }

    if (0 == strcasecmp("NOTICE", value))
    {
        config->log_threshold = LOG_NOTICE;
        return TRUE;
    }

    if (0 == strcasecmp("INFO", value))
    {
        config->log_threshold = LOG_INFO;
        return TRUE;
    }

    if (0 == strcasecmp("DEBUG", value))
    {
        config->log_threshold = LOG_DEBUG;
        return TRUE;
    }

    return FALSE;
}

static int process_use_syslog(const char *value, struct config *config)
{
    if (
            (0 == strcasecmp("TRUE", value)) ||
            (0 == strcasecmp("YES", value)))
    {
        config->log_to_syslog = TRUE;
        return TRUE;
    }

    if (
            (0 == strcasecmp("FALSE", value)) ||
            (0 == strcasecmp("NO", value)))
    {
        config->log_to_syslog = FALSE;
        return TRUE;
    }

    return FALSE;
}

static int process_http_upload_verb(const char *value, struct config *config)
{
    if (
            (0 == strcmp("PUT", value)) ||
            (0 == strcmp("POST", value)) ||
            (0 == strcmp("PATCH", value)))
    {
        strncpy(config->http_upload_verb, value, MAX_UPLOAD_VERB_LEN);
        return TRUE;
    }

    return FALSE;
}

static int process_http_target_url(const char *value, struct config *config)
{
    strncpy(config->http_target_url, value, MAX_TARGET_URL_LEN);
    return TRUE;
}

static int process_http_content_type(const char *value, struct config *config)
{
    strncpy(config->http_content_type, value, MAX_CONTENT_TYPE_LEN);
    return TRUE;
}

static int process_http_json_payload_name(const char *value, struct config *config)
{
    strncpy(config->http_json_payload_name, value, MAX_PAYLOAD_NAME_LEN);
    return TRUE;
}

static int process_param(
        const char* key, const char *value, struct config *config)
{
    if (0 == strcmp("vendor_id", key))
        return process_vendor_id(value, config);

    if (0 == strcmp("product_id", key))
        return process_product_id(value, config);

    if (0 == strcmp("uid", key))
        return process_uid(value, config);

    if (0 == strcmp("gid", key))
        return process_gid(value, config);

    if (0 == strcmp("log_threshold", key))
        return process_log_threshold(value, config);

    if (0 == strcmp("use_syslog", key))
        return process_use_syslog(value, config);

    if (0 == strcmp("http_upload_verb", key))
        return process_http_upload_verb(value, config);

    if (0 == strcmp("http_target_url", key))
        return process_http_target_url(value, config);

    if (0 == strcmp("http_content_type", key))
        return process_http_content_type(value, config);

    if (0 == strcmp("http_json_payload_name", key))
        return process_http_json_payload_name(value, config);

    return FALSE;
}

const char *trim(char* string)
{
    string = find_first_not_of(string, " \"\t\r\n");
    char* last = find_last_not_of(string, " \"\t\r\n");
    if (*last != '\0')
        *(++last) = '\0';
    return string;
}

static int parse_param_line(char* line, struct config *config)
{
    char* equal = strchr(line, '=');
    if (equal == NULL)
    {
        // line doesn't contain a '=' but we expect "key = value"
        return FALSE;
    }

    *equal = '\0';
    const char *key = trim(line);
    const char *value = trim(++equal);

    return process_param(key, value, config);
}

static int parse_raw_line(char* line, struct config *config)
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

    return parse_param_line(line, config);
}

int read_config(struct config *config, const char *filename)
{
    FILE* file = fopen(filename, "r");
    char buffer[MAX_CONFIG_LINE_LEN];
    char* line;

    if (file == NULL)
    {
        return FALSE;
    }

    while ((line = fgets(buffer, MAX_CONFIG_LINE_LEN, file)) != NULL)
    {
        if (!parse_raw_line(line, config))
        {
            return FALSE;
        }
    }

    return TRUE;
}
