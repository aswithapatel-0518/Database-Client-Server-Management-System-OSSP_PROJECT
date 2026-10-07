#ifndef AUTH_H
#define AUTH_H

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_USERS 50

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int active;
} User;

/* User account functions */
int register_user(const char *username, const char *password);
int authenticate_user(const char *username, const char *password);

/* Client session functions */
int login_client(int client_fd, const char *username);
int is_client_logged_in(int client_fd);
void logout_client(int client_fd);
void remove_client_session(int client_fd);

#endif
