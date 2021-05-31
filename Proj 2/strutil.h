#ifndef STRUTIL_H
#define STRUTIL_H

char* unescapeToken(char *token);
char* qtok(char *str, char **next);

// Prints to stderr if debug mode is enabled at compile time. It uses the same formating as printf.
void debug(const char* name, int id, const char* format, ...) __attribute__ ((format (printf, 3, 4)));

#endif
