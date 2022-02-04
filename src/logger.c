#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>

#include "logger.h"

#define MAX_LOG_MSG_LEN     256


static void logger_log_printf(int prio, const char *format, va_list args);
static void logger_log_syslog(int prio, const char *format, va_list args);

static int threshold = LOG_INFO;
static void (*log_function)(int, const char *, va_list ) = &logger_log_printf;

int logger_init(const struct config *config)
{
    threshold = config->log_threshold;

    if (config->log_to_syslog)
    {
        log_function = &logger_log_syslog;
        openlog("barcodes", LOG_CONS, LOG_USER);
    }

    return TRUE;
}

int would_log(int prio)
{
    return prio <= threshold;
}

void logger_log(int prio, const char *format, ...)
{
    va_list args;

    if (prio > threshold)
        return;

    va_start(args, format);
    log_function(prio, format, args);
    va_end(args);
}

void logger_deinit()
{
    if (log_function == logger_log_syslog)
    {
        closelog();
    }
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

static void logger_log_printf(int prio, const char *format, va_list args)
{
    char message[MAX_LOG_MSG_LEN];
    vsnprintf(message, MAX_LOG_MSG_LEN, format, args);

    if (prio <= LOG_WARNING)
    {
        fprintf(stderr, "%s - %s\n", level_name(prio), message);
    }
    else
    {
        fprintf(stdout, "%s - %s\n", level_name(prio), message);
    }
}

static void logger_log_syslog(int prio, const char *format, va_list args)
{
    vsyslog(prio, format, args);
}
