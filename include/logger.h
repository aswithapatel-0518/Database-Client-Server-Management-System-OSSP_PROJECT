#ifndef LOGGER_H
#define LOGGER_H

/* Initialize logging system */
int logger_init(void);

/* Close log files */
void logger_close(void);

/* Write server activity log */
void log_server_activity(const char *message);

/* Write database transaction log */
void log_transaction(
    const char *operation,
    int student_id,
    const char *details
);

#endif
