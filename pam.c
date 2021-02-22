#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <pwd.h>
#include <paths.h>
#include <stdio.h>
#include <unistd.h>

#include "pam.h"

#define SERVICE_NAME "tlm"

#define err(name,r)                                   \
    do {                                            \
        fprintf(stderr, "%s %d : %s\n", name,r,           \
                pam_strerror(pam_handle, result));  \
        end(result);                                \
        return false;                               \
    } while (1);                                    \

static void init_env(struct passwd *pw);
static void set_env(char *name, char *value);
static int end(int last_result);
static int conv(int num_msg, const struct pam_message **msg,
                struct pam_response **resp, void *appdata_ptr);

static pam_handle_t *pam_handle;

bool login(const char *username, const char *password, pid_t *child_pid) {
    const char *data[2] = {username, password};
    struct pam_conv pam_conv = {
        conv, data
    };

    int result = pam_start(SERVICE_NAME, username, &pam_conv, &pam_handle);
    if (result != PAM_SUCCESS) {
        err("pam_start",result);
    }

    result = pam_authenticate(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        err("pam_authenticate",result);
    }

    result = pam_acct_mgmt(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        err("pam_acct_mgmt",result);
    }

    result = pam_setcred(pam_handle, PAM_ESTABLISH_CRED);
    if (result != PAM_SUCCESS) {
        err("pam_setcred",result);
    }

    result = pam_open_session(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        pam_setcred(pam_handle, PAM_DELETE_CRED);
        err("pam_open_session",result);
    }

    struct passwd *pw = getpwnam(username);
    init_env(pw);

    return true;
}

static int conv(int num_msg, const struct pam_message **msg,
                struct pam_response **resp, void *appdata_ptr) {
    int i;

    *resp = calloc(num_msg, sizeof(struct pam_response));
    if (*resp == NULL) {
        return PAM_BUF_ERR;
    }

    int result = PAM_SUCCESS;
    for (i = 0; i < num_msg; i++) {
        char *username, *password;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            username = ((char **) appdata_ptr)[0];
            (*resp)[i].resp = strdup(username);
            break;
        case PAM_PROMPT_ECHO_OFF:
            password = ((char **) appdata_ptr)[1];
            (*resp)[i].resp = strdup(password);
            break;
        case PAM_ERROR_MSG:
            fprintf(stderr, "%s\n", msg[i]->msg);
            result = PAM_CONV_ERR;
            break;
        case PAM_TEXT_INFO:
            printf("%s\n", msg[i]->msg);
            break;
        }

        if (result != PAM_SUCCESS) {
            break;
        }
    }

    if (result != PAM_SUCCESS) {
        free(*resp);
        *resp = 0;
    }

    return result;
}

static void init_env(struct passwd *pw) {
    set_env("HOME", pw->pw_dir);
    set_env("PWD", pw->pw_dir);
    set_env("SHELL", pw->pw_shell);
    set_env("USER", pw->pw_name);
    set_env("LOGNAME", pw->pw_name);
    set_env("PATH", "/usr/local/sbin:/usr/local/bin:/usr/bin");
    set_env("MAIL", _PATH_MAILDIR);
}

static void set_env(char *name, char *value) {
    // The `+ 2` is for the '=' and the null byte
    size_t name_value_len = strlen(name) + strlen(value) + 2;
    char *name_value = malloc(name_value_len);
    snprintf(name_value, name_value_len,  "%s=%s", name, value);
    pam_putenv(pam_handle, name_value);
    free(name_value);
}

bool logout(void) {
    int result = pam_close_session(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        pam_setcred(pam_handle, PAM_DELETE_CRED);
        err("pam_close_session",result);
    }

    result = pam_setcred(pam_handle, PAM_DELETE_CRED);
    if (result != PAM_SUCCESS) {
        err("pam_setcred",result);
    }

    end(result);
    return true;
}

static int end(int last_result) {
    int result = pam_end(pam_handle, last_result);
    pam_handle = 0;
    return result;
}


