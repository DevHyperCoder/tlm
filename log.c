#include "stdio.h"
#include "log.h"

void err(char *message) {
    fprintf(stderr, "%s",message);
}

void info(char *message) {
    fprintf(stdout, "%s",message);
}
