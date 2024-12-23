#include "../include/exam.h"
#include "../include/server.h"
#include "../include/database.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    char line[BUFFER_SIZE];
    while (num_questions < MAX_QUESTIONS && fgets(line, BUFFER_SIZE, file)) {
        Question* q = &questions[num_questions];
        
        // Read subject
        line[strcspn(line, "\n")] = 0;
        strncpy(q->subject, line, sizeof(q->subject) - 1);

        // Read difficulty
        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->difficulty = atoi(line);

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

        num_questions++;
    }

    fclose(file);
    printf("Loaded %d questions\n", num_questions);
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
        // Chế độ training
        printf("Processing training answer '%c' from client %d (question %d)\n", 
               answer, client->fd, client->current_question + 1);

        // Kiểm tra đáp án và tăng điểm
        if (answer == questions[client->question_ids[client->current_question]].correct_answer) {
            client->score++;
            char feedback[BUFFER_SIZE];
            snprintf(feedback, BUFFER_SIZE, "Correct answer!\n");
            send(client->fd, feedback, strlen(feedback), 0);
        } else {
            char feedback[BUFFER_SIZE];
            snprintf(feedback, BUFFER_SIZE, "Wrong answer. The correct answer was %c\n", 
                    questions[client->question_ids[client->current_question]].correct_answer);
            send(client->fd, feedback, strlen(feedback), 0);
        }

        // Chuyển sang câu hỏi tiếp theo
        client->current_question++;

        // Nếu còn câu hỏi thì gửi câu tiếp theo
        if (client->current_question < client->num_questions) {
            send_question(client, client->current_question);
        } else {
            // Kết thúc training
            char buffer[BUFFER_SIZE];
            snprintf(buffer, BUFFER_SIZE, "Training completed! Your final score: %d/%d\n",
                    client->score, client->num_questions);
            send(client->fd, buffer, strlen(buffer), 0);

            // Reset client state
            client->current_question = -1;
            printf("Client %d completed training with score %d/%d\n", 
                   client->fd, client->score, client->num_questions);
        }
        return;
    }

    // Chế độ exam room - giữ nguyên code cũ
    printf("Processing exam answer '%c' from client %d (question %d)\n", 
           answer, client->fd, client->current_question + 1);

    // Kiểm tra đáp án và tăng điểm
    if (answer == questions[room->question_ids[client->current_question]].correct_answer) {
        client->score++;
    }

    // Chuyển sang câu hỏi tiếp theo
    client->current_question++;

    // Nếu còn câu hỏi thì gửi câu tiếp theo
    if (client->current_question < room->num_questions) {
        send_question(client, client->current_question);
    } else {
        // Kết thúc bài thi
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "Exam completed! Your final score: %d/%d\n",
                client->score, room->num_questions);
        send(client->fd, buffer, strlen(buffer), 0);

        // Reset client state
        client->current_question = -1;
        printf("Client %d completed exam with score %d/%d\n", 
               client->fd, client->score, room->num_questions);

        // Lưu kết quả vào file
        save_exam_result(client->username, room->room_id, 
                        client->score, room->num_questions);
    }
}