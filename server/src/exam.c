#include "../include/exam.h"
#include "../include/server.h"
#include "../include/database.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

Question questions[MAX_QUESTIONS];
int num_questions = 0;

void load_questions(void) {
    FILE* file = fopen("questions.txt", "r");
    if (!file) {
        perror("Cannot open questions file");
        return;
    }

    printf("\nStarting to load questions...\n");
    
    char line[BUFFER_SIZE];
    while (num_questions < MAX_QUESTIONS && fgets(line, BUFFER_SIZE, file)) {
        Question* q = &questions[num_questions];
        
        // Read subject - xử lý khoảng trắng
        line[strcspn(line, "\n")] = 0;
        // Bỏ qua dòng trống
        if(strlen(line) == 0) continue;
        
        // Xóa khoảng trắng đầu cuối của subject
        char *start = line;
        while(*start && isspace(*start)) start++;
        char *end = start + strlen(start) - 1;
        while(end > start && isspace(*end)) *end-- = '\0';
        
        strncpy(q->subject, start, sizeof(q->subject) - 1);
        q->subject[sizeof(q->subject) - 1] = '\0';

        // Read difficulty
        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->difficulty = atoi(line);

        // Print debug info
        printf("Loading Question %d:\n", num_questions + 1);
        printf("  Subject: '%s'\n", q->subject);
        printf("  Difficulty: %d\n", q->difficulty);

        // Read question text
        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->question, line, sizeof(q->question) - 1);

        // Read options
        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_A, line, sizeof(q->option_A) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_B, line, sizeof(q->option_B) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_C, line, sizeof(q->option_C) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_D, line, sizeof(q->option_D) - 1);

        // Read correct answer
        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->correct_answer = line[0];

        printf("  Question: %s\n", q->question);
        printf("  Options: A) %s, B) %s, C) %s, D) %s\n", 
               q->option_A, q->option_B, q->option_C, q->option_D);
        printf("  Correct Answer: %c\n\n", q->correct_answer);

        num_questions++;
    }

    fclose(file);
    printf("Successfully loaded %d questions\n", num_questions);

    // In tổng kết theo subject và difficulty
    printf("\nSummary of loaded questions:\n");
    for (int d = 1; d <= 3; d++) {
        printf("Difficulty %d:\n", d);
        char *prev_subject = NULL;
        int count = 0;
        for (int i = 0; i < num_questions; i++) {
            if (questions[i].difficulty == d) {
                if (!prev_subject || strcmp(prev_subject, questions[i].subject) != 0) {
                    if (count > 0) printf("  %s: %d questions\n", prev_subject, count);
                    prev_subject = questions[i].subject;
                    count = 1;
                } else {
                    count++;
                }
            }
        }
        if (prev_subject && count > 0) printf("  %s: %d questions\n", prev_subject, count);
    }
}

void send_question(ClientInfo* client, int question_number) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room) {
        printf("Error: Room not found for client %d\n", client->fd);
        return;
    }

    if (question_number >= room->num_questions) {
        printf("Error: Invalid question number %d\n", question_number);
        return;
    }

    // Kiểm tra và ghi log
    printf("Sending question %d to client %d (username: %s)\n", 
           question_number + 1, client->fd, client->username);

    Question* q = &questions[room->question_ids[question_number]];
    char buffer[BUFFER_SIZE];
    
    snprintf(buffer, BUFFER_SIZE,
             "Question %d/%d\n%s\nA) %s\nB) %s\nC) %s\nD) %s\n",
             question_number + 1, room->num_questions,
             q->question,
             q->option_A, q->option_B, q->option_C, q->option_D);

    if (send(client->fd, buffer, strlen(buffer), 0) < 0) {
        printf("Error sending question to client %d\n", client->fd);
    }
}

void broadcast_to_room(Server* server, ExamRoom* room, const char* message) {
    if (!server || !room || !message) {
        printf("Error: Invalid parameters in broadcast_to_room\n");
        return;
    }

    printf("Broadcasting to room %d: %s\n", room->room_id, message);
    
    for (int i = 0; i < room->user_count; i++) {
        for (int j = 0; j < MAX_CLIENTS; j++) {
            if (server->clients[j].active && 
                strcmp(server->clients[j].username, room->users[i]) == 0) {
                if (send(server->clients[j].fd, message, strlen(message), 0) < 0) {
                    printf("Error broadcasting to client %d\n", server->clients[j].fd);
                }
                break;
            }
        }
    }
}

void start_exam(Server* server, ExamRoom* room) {
    printf("Starting exam in room %d\n", room->room_id);
    room->status = 1;
    room->start_time = time(NULL);
    room->num_questions = NUM_QUESTIONS_PER_EXAM;
    
    // Random chọn câu hỏi
    srand(time(NULL));
    for(int i = 0; i < room->num_questions; i++) {
        room->question_ids[i] = rand() % num_questions;
    }
    
    // Gửi thông báo bắt đầu cho tất cả users trong room
    char start_msg[BUFFER_SIZE];
    snprintf(start_msg, BUFFER_SIZE, "Exam has started! Total questions: %d\n", 
            room->num_questions);

    // Duyệt qua tất cả client trong room
    for (int j = 0; j < MAX_CLIENTS; j++) {
        ClientInfo* client = &server->clients[j];
        if (!client->active) continue;

        // Kiểm tra xem client có trong room không và không phải là chủ room
        for (int i = 0; i < room->user_count; i++) {
            if (strcmp(client->username, room->users[i]) == 0 && 
                strcmp(client->username, room->creator_username) != 0) {
                
                // Gửi thông báo bắt đầu
                send(client->fd, start_msg, strlen(start_msg), 0);
                
                // Khởi tạo trạng thái client
                client->current_question = 0;
                client->score = 0;
                
                // Gửi câu hỏi đầu tiên
                send_question(client, 0);
                break;
            }
        }
    }

    // Gửi thông báo cho chủ room
    for (int j = 0; j < MAX_CLIENTS; j++) {
        ClientInfo* client = &server->clients[j];
        if (client->active && strcmp(client->username, room->creator_username) == 0) {
            char creator_msg[] = "Exam started successfully. Waiting for participants to complete.\n";
            send(client->fd, creator_msg, strlen(creator_msg), 0);
            break;
        }
    }
}

void handle_answer(ClientInfo* client, char answer) {
    ExamRoom* room = get_room(client->current_room_id);
    
    if (!room) {
        return;  // Invalid room, do nothing
    }

    printf("Processing exam answer '%c' from client %d (question %d)\n", 
           answer, client->fd, client->current_question + 1);

    if (answer == questions[room->question_ids[client->current_question]].correct_answer) {
        client->score++;
    }

    client->current_question++;

    if (client->current_question < room->num_questions) {
        send_question(client, client->current_question);
    } else {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "Exam completed! Your final score: %d/%d\n",
                client->score, room->num_questions);
        send(client->fd, buffer, strlen(buffer), 0);

        client->current_question = -1;
        printf("Client %d completed exam with score %d/%d\n", 
               client->fd, client->score, room->num_questions);

        save_exam_result(client->username, room->room_id, 
                        client->score, room->num_questions);
    }
}

void get_available_subjects(char* subjects_list, size_t size) {
    // Đảm bảo có đủ size cho "SUBJECTS|" và "\n"
    size_t remaining_size = size - 10;  // Trừ đi độ dài của "SUBJECTS|" và "\n"
    
    char* subjects[MAX_QUESTIONS];
    int num_subjects = 0;

    // Thu thập unique subjects
    for (int i = 0; i < num_questions; i++) {
        int found = 0;
        for (int j = 0; j < num_subjects; j++) {
            if (strcmp(subjects[j], questions[i].subject) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            subjects[num_subjects++] = questions[i].subject;
        }
    }

    // Đóng gói thành chuỗi, kiểm tra size
    subjects_list[0] = '\0';
    size_t current_length = 0;
    
    for (int i = 0; i < num_subjects; i++) {
        size_t subject_len = strlen(subjects[i]);
        // +1 cho dấu phẩy hoặc null terminator
        if (current_length + subject_len + 1 > remaining_size) {
            break;  // Dừng nếu không đủ space
        }

        if (i > 0) {
            strcat(subjects_list, ",");
            current_length++;
        }
        strcat(subjects_list, subjects[i]);
        current_length += subject_len;
    }
}