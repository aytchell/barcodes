#ifndef LOGGER_H
#define LOGGER_H

#include "config.h"

#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */


int logger_init(const struct config *config);

int would_log(int prio);

void logger_log(int prio, const char *format, ...);

void logger_deinit();

#endif // LOGGER_H
