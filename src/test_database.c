#include <stdio.h>
#include <string.h>

#include "../include/database.h"

int main(void)
{
    if (database_init() != 0) {
        return 1;
    }

    Student s;

    s.id = 101;
    strcpy(s.name, "Aswitha");
    strcpy(s.course, "CSE");
    s.marks = 88.5;
    s.attendance = 92.0;
    s.active = 1;

    printf("\n[TEST] Inserting student...\n");

    if (insert_student(s)) {
        printf("[TEST] Insert successful.\n");
    }
    else {
        printf("[TEST] Insert failed or duplicate.\n");
    }

    printf("\n[TEST] Searching student...\n");

    Student result;

    if (search_student(101, &result)) {
        printf("[TEST] Student found:\n");
        printf("ID: %d\n", result.id);
        printf("Name: %s\n", result.name);
        printf("Course: %s\n", result.course);
        printf("Marks: %.2f\n", result.marks);
        printf("Attendance: %.2f%%\n",
               result.attendance);
    }
    else {
        printf("[TEST] Student not found.\n");
    }

    printf("\n[TEST] Updating student...\n");

    if (update_student(101, 95.0, 96.0)) {
        printf("[TEST] Update successful.\n");
    }

    printf("\n[TEST] Searching after update...\n");

    if (search_student(101, &result)) {
        printf("Updated Marks: %.2f\n", result.marks);
        printf("Updated Attendance: %.2f%%\n",
               result.attendance);
    }

    printf("\n[TEST] Deleting student...\n");

    if (delete_student(101)) {
        printf("[TEST] Delete successful.\n");
    }

    printf("\n[TEST] Searching after delete...\n");

    if (!search_student(101, &result)) {
        printf("[TEST] Student not found. Delete confirmed.\n");
    }

    database_close();

    return 0;
}
