#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void save_exam_result(const char* username, int room_id, int score, int total) {
    FILE* file = fopen(RESULTS_FILE, "a");
    if (!file) {
        printf("Error: Cannot open results file\n");
        return;
    }

    // Lấy thời gian hiện tại
    time_t rawtime;
    struct tm *timeinfo;
    char time_buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y %H:%M:%S", timeinfo);

    // Ghi theo định dạng: User: Duc, Room: 1, Score: 10/10, Time: 20-12-2023 15:30:45
    fprintf(file, "User: %s, Room ID: %d, Score/Total: %d/%d, Execution Time: %s\n",
            username, room_id, score, total, time_buffer);

    fclose(file);
    printf("Saved exam result for user %s\n", username);
}

void get_user_results(const char* username) {
    FILE* file = fopen(RESULTS_FILE, "r");
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