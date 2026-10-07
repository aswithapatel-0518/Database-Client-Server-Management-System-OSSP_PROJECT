#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "../include/auth.h"
#include "../include/common.h"

#define USER_FILE "data/users.dat"

typedef struct {
    int client_fd;
    int logged_in;
    char username[MAX_USERNAME];
} ClientSession;

static ClientSession sessions[MAX_CLIENTS];

static pthread_mutex_t auth_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;


/* Register a new user */
int register_user(const char *username, const char *password)
{
    pthread_mutex_lock(&auth_mutex);

    FILE *file = fopen(USER_FILE, "a+");

    if (file == NULL) {
        pthread_mutex_unlock(&auth_mutex);
        return -1;
    }

    User user;

    rewind(file);

    while (fscanf(
        file,
        "%49s %49s",
        user.username,
        user.password
    ) == 2) {

        if (strcmp(user.username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&auth_mutex);
            return 1;
        }
    }

    fprintf(file, "%s %s\n", username, password);

    fclose(file);

    pthread_mutex_unlock(&auth_mutex);

    return 0;
}


/* Authenticate user */
int authenticate_user(const char *username, const char *password)
{
    pthread_mutex_lock(&auth_mutex);

    FILE *file = fopen(USER_FILE, "r");

    if (file == NULL) {
        pthread_mutex_unlock(&auth_mutex);
        return -1;
    }

    User user;

    while (fscanf(
        file,
        "%49s %49s",
        user.username,
        user.password
    ) == 2) {

        if (strcmp(user.username, username) == 0 &&
            strcmp(user.password, password) == 0) {

            fclose(file);
            pthread_mutex_unlock(&auth_mutex);

            return 0;
        }
    }

    fclose(file);

    pthread_mutex_unlock(&auth_mutex);

    return 1;
}


/* Mark a client as logged in */
int login_client(int client_fd, const char *username)
{
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (sessions[i].client_fd == client_fd) {

            sessions[i].logged_in = 1;

            strncpy(
                sessions[i].username,
                username,
                MAX_USERNAME - 1
            );

            sessions[i].username[MAX_USERNAME - 1] = '\0';

            pthread_mutex_unlock(&session_mutex);

            return 0;
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (sessions[i].client_fd == 0) {

            sessions[i].client_fd = client_fd;
            sessions[i].logged_in = 1;

            strncpy(
                sessions[i].username,
                username,
                MAX_USERNAME - 1
            );

            sessions[i].username[MAX_USERNAME - 1] = '\0';

            pthread_mutex_unlock(&session_mutex);

            return 0;
        }
    }

    pthread_mutex_unlock(&session_mutex);

    return -1;
}


/* Check whether client is logged in */
int is_client_logged_in(int client_fd)
{
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (sessions[i].client_fd == client_fd) {

            int logged_in = sessions[i].logged_in;

            pthread_mutex_unlock(&session_mutex);

            return logged_in;
        }
    }

    pthread_mutex_unlock(&session_mutex);

    return 0;
}


/* Logout client */
void logout_client(int client_fd)
{
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (sessions[i].client_fd == client_fd) {
            sessions[i].logged_in = 0;
            sessions[i].username[0] = '\0';
            break;
        }
    }

    pthread_mutex_unlock(&session_mutex);
}


/* Remove client session completely */
void remove_client_session(int client_fd)
{
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (sessions[i].client_fd == client_fd) {

            sessions[i].client_fd = 0;
            sessions[i].logged_in = 0;
            sessions[i].username[0] = '\0';

            break;
        }
    }

    pthread_mutex_unlock(&session_mutex);
}
