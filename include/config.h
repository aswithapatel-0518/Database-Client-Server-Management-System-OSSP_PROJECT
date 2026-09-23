#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int port;
    int workers;
    int max_clients;
    int log_level;
} Config;

extern Config server_config;

int load_config(void);
void print_config(void);

#endif
