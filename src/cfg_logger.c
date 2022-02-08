#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "cfg_logger.h"
#include "logger.h"

#define MAX_LOG_LINE_LEN    256

struct log_entry
{
    int log_level;
    struct log_entry *next;
    char message[MAX_LOG_LINE_LEN];
};

struct config_logger
{
    struct log_entry *first;
    struct log_entry *last;
};

struct config_logger *cfg_logger_new()
{
    struct config_logger *logger =
        (struct config_logger*)malloc(sizeof(struct config_logger));
    logger->first = NULL;
    logger->last = NULL;
    return logger;
}

void cfg_logger_flush(struct config_logger *logger)
{
    if (logger == NULL)
        return;

    struct log_entry *entry = logger->first;
    while (entry != NULL)
    {
        logger_log(entry->log_level, "%s", entry->message);
        entry = entry->next;
    }
}

void cfg_logger_delete(struct config_logger *logger)
{
    if (logger == NULL)
        return;

    struct log_entry *entry = logger->first;
    while (entry != NULL)
    {
        struct log_entry *next = entry->next;
        free(entry);
        entry = next;
    }
}

void cfg_logger_log(struct config_logger *logger, int level,
        const char *format, ...)
{
    if (logger == NULL)
        return;

    struct log_entry *entry = (struct log_entry*)malloc(
            sizeof(struct log_entry));
    entry->log_level = level;
    entry->next = NULL;

    if (logger->last == NULL)
    {
        logger->first = entry;
    } else {
        logger->last->next = entry;
    }
    logger->last = entry;

    va_list args;
    va_start(args, format);
    vsnprintf(entry->message, MAX_LOG_LINE_LEN, format, args);
    va_end(args);
}
