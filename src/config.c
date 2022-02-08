#include "config.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>

#define SCANNER_VENDOR_ID   0x28e9
#define SCANNER_PRODUCT_ID  0x03d9

#define NONPRIV_GROUP_ID    1000
#define NONPRIV_USER_ID     1000

#define NEVER_TIMEOUT       -1
#define JAMBEL_EVENT_URL    "http://localhost:8080/event-input/558a4918-54a7-492c-b268-3790f4d5f0f5"
#define HTTP_CONTENT_TYPE   "application/vnd.com.github.aytchell.eventvalue-v1+json"

#define MAX_CONFIG_LINE_LEN 768

void set_defaults(struct config *config)
{
    config->scanner_vendor_id = SCANNER_VENDOR_ID;
    config->scanner_product_id = SCANNER_PRODUCT_ID;
    config->nonpriv_gid = NONPRIV_GROUP_ID;
    config->nonpriv_uid = NONPRIV_USER_ID;
    config->scan_timeout = NEVER_TIMEOUT;
    config->log_threshold = LOG_DEBUG;
    config->log_to_syslog = FALSE;
    strncpy(config->http_upload_verb, "PUT", MAX_UPLOAD_VERB_LEN);
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

static int process_param(const char* key, const char *value,
        __attribute__((unused)) struct config *config)
{
    fprintf(stdout, "Key: '%s' - Value: '%s'\n", key, value);
    return TRUE;
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
