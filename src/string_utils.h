#ifndef STRING_UTILS_H
#define STRING_UTILS_H


int matches_any_of(char current, char* patterns);

char* find_first_not_of(char *line, char *patterns);

char* find_last_not_of(char *line, char *patterns);

const char *trim(char* string);

int parse_uint16(const char *value);


#endif // STRING_UTILS_H
