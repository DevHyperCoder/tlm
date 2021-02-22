#ifndef _PAM_H
#define _PAM_H

#include <stdbool.h>
#include <sys/types.h>

bool login(const char *username,const char *passwd, pid_t *child_pid);
bool logout(void);

#endif
