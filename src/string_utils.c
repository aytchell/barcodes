#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "string_utils.h"


int matches_any_of(char current, char* patterns)
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

char* find_first_not_of(char *line, char *patterns)
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

char* find_last_not_of(char *line, char *patterns)
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

int parse_uint16(const char *value)
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

const char *trim(char* string)
{
    string = find_first_not_of(string, " \"\t\r\n");
    char* last = find_last_not_of(string, " \"\t\r\n");
    if (*last != '\0')
        *(++last) = '\0';
    return string;
}
