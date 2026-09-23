#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include <pthread.h>

extern pthread_mutex_t db_mutex;

int database_init(void);
void database_close(void);

int insert_student(Student s);
int search_student(int id, Student *result);
int update_student(int id, float marks, float attendance);
int delete_student(int id);

void display_all_students(void);

#endif
