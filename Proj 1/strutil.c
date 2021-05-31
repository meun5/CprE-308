#include <assert.h>
#include <ctype.h>

/**
 * NOTICE: This code is an adaptation of code found on internet.
 * It has been adapted to fit this project, and to edit the code style.
 *
 * Source: https://stackoverflow.com/a/33780304
 * Original Author: Calmarius https://stackoverflow.com/users/58805/calmarius
 * Original License: CC BY-SA 3.0 https://creativecommons.org/licenses/by-sa/3.0/
 */

// Strips backslashes from quotes
char *unescapeToken(char *token) {
    char *in = token;
    char *out = token;

    while (*in) {
        if (in <= out) {
            return out;
        }

        if ((in[0] == '\\') && (in[1] == '"')) {
            *out = in[1];
            out++;
            in += 2;
        } else {
            *out = *in;
            out++;
            in++;
        }
    }

    *out = 0;

    return token;
}

// Returns the end of the token, without changing it.
char *qtok(char *str, char **next) {
    char *current = str;
    char *start = str;
    int isQuoted = 0;

    // Eat beginning whitespace.
    while (*current && isspace(*current)) {
        current++;
    }

    start = current;

    if (*current == '"') {
        isQuoted = 1;

        // Quoted token
        current++; // Skip the beginning quote.
        start = current;
        for (;;) {
            // Go till we find a quote or the end of string.
            while (*current && (*current != '"')) {
                current++;
            }

            if (!*current) {
                // Reached the end of the string.
                break;
            }

            if (*(current - 1) == '\\') {
                // Escaped quote keep going.
                current++;
                continue;
            }
            // Reached the ending quote.
            break;
        }
    } else {
        // Not quoted so run till we see a space.
        while (*current && !isspace(*current)) {
            current++;
        }
    }

    if (*current) {
        // Close token if not closed already.
        *current = 0;
        current++;
        // Eat trailing whitespace.
        while (*current && isspace(*current)) {
            current++;
        }
    }

    *next = current;

    return isQuoted ? unescapeToken(start) : start;
}
// END NOTICE
