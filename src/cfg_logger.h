#ifndef CONFIG_LOGGER_H
#define CONFIG_LOGGER_H

struct config_logger;

struct config_logger *cfg_logger_new();

void cfg_logger_flush(struct config_logger *logger);

void cfg_logger_delete(struct config_logger *logger);

void cfg_logger_log(struct config_logger *logger, int prio,
        const char *format, ...);

#endif // CONFIG_LOGGER_H
