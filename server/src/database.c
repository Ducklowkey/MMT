#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void save_exam_result(const char* username, int room_id, int score, int total) {
    FILE* file = fopen("results.txt", "a");
    if (!file) {
        printf("Error: Cannot open results file\n");
        return;
    }

    // Ghi theo định dạng: username,room_id,score,total,timestamp
    fprintf(file, "%s,%d,%d,%d,%ld\n",
            username, room_id, score, total, time(NULL));

    fclose(file);
    printf("Saved exam result for user %s\n", username);
}

void get_user_results(const char* username) {
    FILE* file = fopen("results.txt", "r");
    if (!file) {
        printf("No results found\n");
        return;
    }

    char line[BUFFER_SIZE];
    int found = 0;
    printf("\nExam results for %s:\n", username);

    while (fgets(line, sizeof(line), file)) {
        char stored_username[MAX_USERNAME];
        int room_id, score, total;
        time_t timestamp;

        // Đọc từng dòng theo định dạng đã lưu
        if (sscanf(line, "%[^,],%d,%d,%d,%ld",
                   stored_username, &room_id, &score, &total, &timestamp) == 5) {
            if (strcmp(stored_username, username) == 0) {
                found = 1;
                printf("Room %d: Score %d/%d (taken at: %s)",
                       room_id, score, total, ctime(&timestamp));
            }
        }
    }

    if (!found) {
        printf("No exam results found for %s\n", username);
    }

    fclose(file);
}