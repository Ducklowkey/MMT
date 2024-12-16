// server/src/database.c
#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DB_FILE "exam_results.db"

void init_database(void) {
    FILE* file = fopen(DB_FILE, "a+");
    if (!file) {
        perror("Cannot create/open database file");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

void get_user_results(const char* username, ExamResult** results, int* count) {
    FILE* file = fopen(DB_FILE, "rb");
    if (!file) {
        *results = NULL;
        *count = 0;
        return;
    }

    // Count matching results first
    ExamResult temp;
    *count = 0;
    while (fread(&temp, sizeof(ExamResult), 1, file)) {
        if (strcmp(temp.username, username) == 0) {
            (*count)++;
        }
    }

    if (*count == 0) {
        fclose(file);
        *results = NULL;
        return;
    }

    // Allocate memory for results
    *results = malloc(sizeof(ExamResult) * (*count));
    if (!*results) {
        perror("Memory allocation failed");
        fclose(file);
        *count = 0;
        return;
    }

    // Read matching results
    rewind(file);
    int index = 0;
    while (fread(&temp, sizeof(ExamResult), 1, file)) {
        if (strcmp(temp.username, username) == 0) {
            (*results)[index++] = temp;
        }
    }

    fclose(file);
}

void close_database(void) {
    // Cleanup any open database connections if needed
}

// Function to calculate exam statistics
void calculate_statistics(const char* username, int* total_exams,
                        float* avg_score, int* highest_score) {
    ExamResult* results;
    int count;
    get_user_results(username, &results, &count);

    if (count == 0 || !results) {
        *total_exams = 0;
        *avg_score = 0.0f;
        *highest_score = 0;
        return;
    }

    *total_exams = count;
    float total_score = 0;
    *highest_score = results[0].score;

    for (int i = 0; i < count; i++) {
        total_score += results[i].score;
        if (results[i].score > *highest_score) {
            *highest_score = results[i].score;
        }
    }

    *avg_score = total_score / count;
    free(results);
}

// Function to get recent exam results
void get_recent_results(ExamResult** results, int* count, int limit) {
    FILE* file = fopen(DB_FILE, "rb");
    if (!file) {
        *results = NULL;
        *count = 0;
        return;
    }

    // Get total number of results
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    *count = file_size / sizeof(ExamResult);

    if (*count == 0) {
        fclose(file);
        *results = NULL;
        return;
    }

    // Limit the number of results if needed
    if (limit > 0 && *count > limit) {
        *count = limit;
    }

    // Allocate memory for results
    *results = malloc(sizeof(ExamResult) * (*count));
    if (!*results) {
        perror("Memory allocation failed");
        fclose(file);
        *count = 0;
        return;
    }

    // Read most recent results
    fseek(file, -(*count * sizeof(ExamResult)), SEEK_END);
    fread(*results, sizeof(ExamResult), *count, file);
    fclose(file);
}
