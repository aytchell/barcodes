#include <stdio.h>
#include <stdarg.h>

#include "logger.h"

#define MAX_LOG_MSG_LEN     256

static int threshold = LOG_INFO;

int logger_init(const struct config *config)
{
    threshold = config->log_threshold;
    return TRUE;
}

static const char* level_name(int prio)
{
    switch (prio)
    {
        case LOG_EMERG:     return "EMRG";
        case LOG_ALERT:     return "ALRT";
        case LOG_CRIT:      return "CRIT";
        case LOG_ERR:       return "ERR ";
        case LOG_WARNING:   return "WARN";
        case LOG_NOTICE:    return "NOTE";
        case LOG_INFO:      return "INFO";
        case LOG_DEBUG:     return "DEBG";
        default:            return "????";
    }
}

int would_log(int prio)
{
    return prio <= threshold;
}

void logger_log(int prio, const char *format, ...)
{
    char message[MAX_LOG_MSG_LEN];
    va_list args;

    if (prio > threshold)
        return;

    va_start(args, format);
    vsnprintf(message, MAX_LOG_MSG_LEN, format, args);
    va_end(args);

    if (prio <= LOG_WARNING)
    {
        fprintf(stderr, "%s - %s\n", level_name(prio), message);
    }
    else
    {
        fprintf(stdout, "%s - %s\n", level_name(prio), message);
    }
}

void logger_deinit()
{
}
