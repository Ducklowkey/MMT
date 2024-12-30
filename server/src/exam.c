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
        perror("Không thể mở file câu hỏi");
        return;
    }

    printf("\nBắt đầu load câu hỏi...\n");
    
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

        //Đọc thông tin câu hỏi 

        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->difficulty = atoi(line);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->question, line, sizeof(q->question) - 1);

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

        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->correct_answer = line[0];

        num_questions++;
    }

    fclose(file);
    printf("Load thành công %d câu hỏi\n", num_questions);

    // In tổng kết theo subject và difficulty
    printf("\nHệ thống số câu hỏi đang có :\n");
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
    
    // Tạo mảng chỉ số cho từng độ khó
    int easy_questions[MAX_QUESTIONS];
    int medium_questions[MAX_QUESTIONS];
    int hard_questions[MAX_QUESTIONS];
    int easy_count = 0, medium_count = 0, hard_count = 0;
    
    // Lọc và phân loại câu hỏi theo môn học và độ khó
    for (int i = 0; i < num_questions; i++) {
        if (strstr(room->subjects, questions[i].subject) || strstr(room->subjects, "All")) {
            switch (questions[i].difficulty) {
                case 1: 
                    easy_questions[easy_count++] = i;
                    break;
                case 2:
                    medium_questions[medium_count++] = i;
                    break;
                case 3:
                    hard_questions[hard_count++] = i;
                    break;
            }
        }
    }
    
    // Điều chỉnh số lượng câu hỏi nếu không đủ
    char adjust_msg[BUFFER_SIZE];
    if (room->num_easy > easy_count) {
        snprintf(adjust_msg, BUFFER_SIZE, "Do không đủ số lượng câu hỏi, điều chỉnh số câu hỏi dễ từ %d xuống %d...\n",
                room->num_easy, easy_count);
        broadcast_to_room(server, room, adjust_msg);
        room->num_easy = easy_count;
    }
    if (room->num_medium > medium_count) {
        snprintf(adjust_msg, BUFFER_SIZE, "Do không đủ số lượng câu hỏi, điều chỉnh số câu hỏi trung bình từ %d xuống %d...\n",
                room->num_medium, medium_count);
        broadcast_to_room(server, room, adjust_msg);
        room->num_medium = medium_count;
    }
    if (room->num_hard > hard_count) {
        snprintf(adjust_msg, BUFFER_SIZE, "Do không đủ số lượng câu hỏi, điều chỉnh số câu hỏi khó từ %d xuống %d...\n",
                room->num_hard, hard_count);
        broadcast_to_room(server, room, adjust_msg);
        room->num_hard = hard_count;
    }

    // Cập nhật tổng số câu hỏi
    int total_available = room->num_easy + room->num_medium + room->num_hard;
    if (total_available == 0) {
        char error_msg[BUFFER_SIZE];
        snprintf(error_msg, BUFFER_SIZE, "Không tìm thấy câu hỏi phù hợp với các tiêu chí đã chọn.\n");
        broadcast_to_room(server, room, error_msg);
        room->status = 0;
        return;
    }

    room->num_questions = total_available;

    // Chọn ngẫu nhiên câu hỏi và lưu vị trí thực tế
    srand(time(NULL));
    int question_index = 0;

    // Chọn câu hỏi dễ
    for (int i = 0; i < room->num_easy; i++) {
        int random_easy = rand() % easy_count;
        room->question_ids[question_index++] = easy_questions[random_easy];
    }

    // Chọn câu hỏi trung bình
    for (int i = 0; i < room->num_medium; i++) {
        int random_medium = rand() % medium_count;
        room->question_ids[question_index++] = medium_questions[random_medium];
    }

    // Chọn câu hỏi khó
    for (int i = 0; i < room->num_hard; i++) {
        int random_hard = rand() % hard_count;
        room->question_ids[question_index++] = hard_questions[random_hard];
    }

    room->num_questions = question_index;  // Cập nhật tổng số câu hỏi

    // Xáo trộn thứ tự câu hỏi
    for (int i = room->num_questions - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = room->question_ids[i];
        room->question_ids[i] = room->question_ids[j];
        room->question_ids[j] = temp;
    }
    
    // Gửi thông báo bắt đầu cho tất cả users trong room
    char start_msg[BUFFER_SIZE];
    snprintf(start_msg, BUFFER_SIZE, "Exam has started! Total questions: %d (Easy: %d, Medium: %d, Hard: %d)\n", 
            room->num_questions, room->num_easy, room->num_medium, room->num_hard);

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
            char creator_msg[BUFFER_SIZE];
            snprintf(creator_msg, BUFFER_SIZE, 
                    "Exam started successfully. Configuration:\n"
                    "- Total questions: %d\n"
                    "- Easy: %d\n"
                    "- Medium: %d\n"
                    "- Hard: %d\n"
                    "- Time limit: %d minutes\n"
                    "- Subjects: %s\n"
                    "Waiting for participants to complete.\n",
                    room->num_questions, room->num_easy, room->num_medium, 
                    room->num_hard, room->time_limit/60, room->subjects);
            send(client->fd, creator_msg, strlen(creator_msg), 0);
            break;
        }
    }
}

void handle_answer(ClientInfo* client, char answer) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room) return;

    printf("Processing exam answer '%c' from client %d (question %d)\n", 
           answer, client->fd, client->current_question + 1);

    // Lưu câu trả lời
    client->answers[client->current_question] = answer;
    client->question_answered[client->current_question] = 1;

    if (answer == questions[room->question_ids[client->current_question]].correct_answer) {
        client->score++;
    }

    client->current_question++;

    if (client->current_question < room->num_questions) {
        send_question(client, client->current_question);
    } else {
        show_review_menu(client);  // Hiển thị menu xem lại
    }
}

void get_available_subjects(char* subjects_list, size_t size) {
    char* subjects[MAX_QUESTIONS];
    int num_subjects = 0;

    // Thu thập subjects từ questions
    for (int i = 0; i < num_questions; i++) {
        int found = 0;
        for (int j = 0; j < num_subjects; j++) {
            if (strcmp(subjects[j], questions[i].subject) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            subjects[num_subjects++] = strdup(questions[i].subject);
        }
    }

    // Đóng gói thành chuỗi
    subjects_list[0] = '\0';
    for (int i = 0; i < num_subjects; i++) {
        strcat(subjects_list, subjects[i]);
        if (i < num_subjects - 1) {
            strcat(subjects_list, ",");
        }
        free(subjects[i]);
    }
}

// Kiểm tra thời gian còn lại
int is_exam_time_remaining(ExamRoom* room) {
    if (!room) return 0;
    time_t current_time = time(NULL);
    // Mặc định cho 1 tiếng thi
    return (current_time - room->start_time) < 3600;
}

// Gửi thời gian còn lại
void send_time_remaining(ClientInfo* client) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room) return;

    time_t current_time = time(NULL);
    int time_left = 3600 - (current_time - room->start_time);
    if (time_left < 0) time_left = 0;

    char time_message[BUFFER_SIZE];
    snprintf(time_message, sizeof(time_message), "TIME_LEFT: %d seconds\n", time_left);
    send(client->fd, time_message, strlen(time_message), 0);
}

// Xử lý nộp bài sớm và tính điểm
void handle_exam_submit(ClientInfo* client) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room) return;

    int score = client->score;
    int total_questions = room->num_questions;

    char result_message[BUFFER_SIZE];
    snprintf(result_message, sizeof(result_message), 
             "Exam completed! Your final score: %d/%d\n",
             score, total_questions);
    send(client->fd, result_message, strlen(result_message), 0);
    
    save_exam_result(client->username, room->room_id, score, total_questions);

    // Reset trạng thái
    client->in_review_mode = 0;
    client->current_question = -1;
}

void show_review_menu(ClientInfo* client) {
    char buffer[BUFFER_SIZE];
    ExamRoom* room = get_room(client->current_room_id);
    if (!room) return;

    snprintf(buffer, BUFFER_SIZE, 
            "\n===== CHẾ ĐỘ XEM LẠI =====\n"
            "REVIEW <số câu>: xem lại câu hỏi\n"
            "CHANGE <số câu> <đáp án>: sửa đáp án\n"
            "TIME: xem thời gian còn lại\n"
            "SUBMIT: nộp bài\n\n");
    send(client->fd, buffer, strlen(buffer), 0);
}

void handle_review_request(ClientInfo* client, int question_num) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room || question_num <= 0 || question_num > room->num_questions) {
        send(client->fd, "Số câu hỏi không hợp lệ.\n", strlen("Số câu hỏi không hợp lệ.\n"), 0);
        return;
    }

    Question* q = &questions[room->question_ids[question_num - 1]];
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE,
             "\nCâu %d:\n%s\nA) %s\nB) %s\nC) %s\nD) %s\n"
             "Đáp án đã chọn: %c\n",
             question_num, q->question,
             q->option_A, q->option_B, q->option_C, q->option_D,
             client->question_answered[question_num - 1] ? client->answers[question_num - 1] : '-');
    send(client->fd, buffer, strlen(buffer), 0);
}

void handle_change_answer(ClientInfo* client, int question_num, char new_answer) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room || question_num <= 0 || question_num > room->num_questions) {
        send(client->fd, "Số câu hỏi không hợp lệ.\n", strlen("Số câu hỏi không hợp lệ.\n"), 0);
        return;
    }

    // Cập nhật đáp án và điểm
    int question_idx = question_num - 1;
    if (client->question_answered[question_idx]) {
        if (client->answers[question_idx] == questions[room->question_ids[question_idx]].correct_answer) {
            client->score--;
        }
    }

    client->answers[question_idx] = new_answer;
    client->question_answered[question_idx] = 1;

    if (new_answer == questions[room->question_ids[question_idx]].correct_answer) {
        client->score++;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "Đã thay đổi đáp án câu %d thành %c.\n", 
             question_num, new_answer);
    send(client->fd, buffer, strlen(buffer), 0);

    // Thêm flag đánh dấu đang trong chế độ xem lại
    client->in_review_mode = 1;
    // Reset current_question về -1 để tránh xử lý như câu hỏi tiếp theo
    client->current_question = -1;

    show_review_menu(client);
}