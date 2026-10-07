#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include "../include/database.h"
#include "../include/logger.h"

#define DB_FILE "data/students.dat"

static Student *db = NULL;
static int db_fd = -1;

pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;


/* Initialize memory-mapped database */
int database_init(void)
{
    db_fd = open(DB_FILE, O_RDWR | O_CREAT, 0666);

    if (db_fd == -1) {
        perror("Database open failed");
        return -1;
    }

    size_t db_size = sizeof(Student) * MAX_STUDENTS;

    if (ftruncate(db_fd, db_size) == -1) {
        perror("ftruncate failed");
        close(db_fd);
        db_fd = -1;
        return -1;
    }

    db = mmap(
        NULL,
        db_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        db_fd,
        0
    );

    if (db == MAP_FAILED) {
        perror("mmap failed");
        close(db_fd);
        db_fd = -1;
        db = NULL;
        return -1;
    }

    log_server_activity("Memory-mapped database initialized");

    return 0;
}


/* Close and unmap database */
void database_close(void)
{
    if (db != NULL && db != MAP_FAILED) {
        msync(
            db,
            sizeof(Student) * MAX_STUDENTS,
            MS_SYNC
        );

        munmap(
            db,
            sizeof(Student) * MAX_STUDENTS
        );

        db = NULL;
    }

    if (db_fd != -1) {
        close(db_fd);
        db_fd = -1;
    }

    log_server_activity("Memory-mapped database closed");
}


/* Insert a student */
int insert_student(Student student)
{
    pthread_mutex_lock(&db_mutex);

    for (int i = 0; i < MAX_STUDENTS; i++) {

        if (db[i].active == 1 && db[i].id == student.id) {
            pthread_mutex_unlock(&db_mutex);

            log_transaction(
                "INSERT_FAILED",
                student.id,
                "Student ID already exists"
            );

            return -1;
        }
    }

    for (int i = 0; i < MAX_STUDENTS; i++) {

        if (db[i].active == 0) {
            db[i] = student;
            db[i].active = 1;

            msync(
                db,
                sizeof(Student) * MAX_STUDENTS,
                MS_SYNC
            );

            pthread_mutex_unlock(&db_mutex);

            log_transaction(
                "INSERT",
                student.id,
                "Student inserted successfully"
            );

            return 0;
        }
    }

    pthread_mutex_unlock(&db_mutex);

    log_transaction(
        "INSERT_FAILED",
        student.id,
        "Database is full"
    );

    return -1;
}


/* Search for a student */
int search_student(int id, Student *result)
{
    if (result == NULL) {
        return -1;
    }

    pthread_mutex_lock(&db_mutex);

    for (int i = 0; i < MAX_STUDENTS; i++) {

        if (db[i].active == 1 && db[i].id == id) {
            *result = db[i];

            pthread_mutex_unlock(&db_mutex);

            log_transaction(
                "SEARCH",
                id,
                "Student found"
            );

            return 0;
        }
    }

    pthread_mutex_unlock(&db_mutex);

    log_transaction(
        "SEARCH_FAILED",
        id,
        "Student not found"
    );

  return -1;
}


/* Update a student */
/* Update a student's marks and attendance */
int update_student(int id, float marks, float attendance)
{
    pthread_mutex_lock(&db_mutex);

    for (int i = 0; i < MAX_STUDENTS; i++) {

        if (db[i].active == 1 && db[i].id == id) {

            db[i].marks = marks;
            db[i].attendance = attendance;

            msync(
                db,
                sizeof(Student) * MAX_STUDENTS,
                MS_SYNC
            );

            pthread_mutex_unlock(&db_mutex);

            log_transaction(
                "UPDATE",
                id,
                "Student marks and attendance updated successfully"
            );

            return 0;
        }
    }

    pthread_mutex_unlock(&db_mutex);

    log_transaction(
        "UPDATE_FAILED",
        id,
        "Student not found"
    );

    return -1;
}


/* Delete a student */
int delete_student(int id)
{
    pthread_mutex_lock(&db_mutex);

    for (int i = 0; i < MAX_STUDENTS; i++) {

        if (db[i].active == 1 && db[i].id == id) {
            db[i].active = 0;

            msync(
                db,
                sizeof(Student) * MAX_STUDENTS,
                MS_SYNC
            );

            pthread_mutex_unlock(&db_mutex);

            log_transaction(
                "DELETE",
                id,
                "Student deleted successfully"
            );

            return 0;
        }
    }

    pthread_mutex_unlock(&db_mutex);

    log_transaction(
        "DELETE_FAILED",
        id,
        "Student not found"
    );

    return -1;
}


/* Display all active students */
void display_all_students(char *output, size_t output_size)
{
    pthread_mutex_lock(&db_mutex);

    size_t used = 0;
    int found = 0;

    used += snprintf(
        output + used,
        output_size - used,
        "========== STUDENT DATABASE ==========\n"
    );

    for (int i = 0; i < MAX_STUDENTS; i++) {

        if (db[i].active == 1) {

            found = 1;

            used += snprintf(
                output + used,
                output_size - used,
                "ID: %d | Name: %s | Course: %s | Marks: %.2f | Attendance: %.2f%%\n",
                db[i].id,
                db[i].name,
                db[i].course,
                db[i].marks,
                db[i].attendance
            );

            if (used >= output_size) {
                break;
            }
        }
    }

    if (!found) {
        used += snprintf(
            output + used,
            output_size - used,
            "No active students found.\n"
        );
    }

    snprintf(
        output + (used < output_size ? used : output_size - 1),
        used < output_size ? output_size - used : 1,
        "======================================\n"
    );

    pthread_mutex_unlock(&db_mutex);
}
