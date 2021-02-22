#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char* trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace(*str))
        str++;

    // All spaces
    if(*str==0)
        return str;

    end = str + strnlen(str,128)-1;

    while(end > str && isspace(*end))
        end --;

    *(end+1) = '\0';

    return str;
}
