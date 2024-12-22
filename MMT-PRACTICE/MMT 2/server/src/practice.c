#include "../include/exam.h"
#include "../include/room.h"
#include "../include/practice.h"
#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Khai báo mảng câu hỏi và biến số lượng câu hỏi
Question questions_practice[MAX_QUESTIONS];
int num_questions_practice = 0;
ClientData client_data[MAX_CLIENTS]; // Mảng chứa dữ liệu của tất cả các client
// Hàm nạp câu hỏi từ file
void load_questions_practices(void) {
    FILE* file = fopen("questions.txt", "r");
    if (!file) {
        perror("Cannot open questions file");
        return;
    }
    
    char line[BUFFER_SIZE];
    while (num_questions_practice < MAX_QUESTIONS && fgets(line, BUFFER_SIZE, file)) {
        Question* q = &questions_practice[num_questions_practice];

        line[strcspn(line, "\n")] = 0;
        strncpy(q->subject, line, sizeof(q->subject) - 1);

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

        num_questions_practice++;
    }

    fclose(file);
    printf("Loaded %d questions\n", num_questions_practice);
}

// Hàm lọc câu hỏi theo thông số đã chọn, hỗ trợ nhiều client
void filter_questions(Question* filtered_questions, int* filtered_count, int difficulty, const char* subjects, int client_socket) {
    *filtered_count = 0;
    
    // Vòng lặp qua tất cả câu hỏi
    for (int i = 0; i < num_questions_practice; i++) {
        // Lọc theo độ khó
        if (questions_practice[i].difficulty != difficulty) continue;

        // Lọc theo môn học
        if (strstr(subjects, "All") || strstr(subjects, questions_practice[i].subject)) {
            // Kiểm tra nếu câu hỏi này đã được chọn cho client khác
            int is_already_assigned = 0;
            for (int j = 0; j < *filtered_count; j++) {
                if (strcmp(filtered_questions[j].question, questions_practice[i].question) == 0) {
                    is_already_assigned = 1;
                    break;
                }
            }

            // Nếu câu hỏi chưa được chọn, thêm vào danh sách của client này
            if (!is_already_assigned) {
                filtered_questions[*filtered_count] = questions_practice[i];
                (*filtered_count)++;
            }
        }
    }
    // In thông báo số câu hỏi lọc được cho client
    printf("Client %d: Filtered %d questions\n", client_socket, *filtered_count);
}

// Hàm tính điểm cho chế độ luyện tập, hỗ trợ nhiều client
int calculate_score_practice(const char* answers, const Question* questions, int num_questions, int client_socket) {
    int score = 0;

    // Vòng lặp qua từng câu hỏi và so sánh câu trả lời của client với đáp án đúng
    for (int i = 0; i < num_questions; i++) {
        if (answers[i] == questions[i].correct_answer) {
            score++;
        }
    }

    // Lưu điểm cho client vào mảng client_data
    client_data[client_socket].score = score;
    printf("Client %d: Final score = %d/%d\n", client_socket, score, num_questions);
    return score;
}

// Hàm gửi kết quả thi cho client
void send_result_to_client(int score, int num_questions, int client_socket) {
    char result_msg[BUFFER_SIZE];
    snprintf(result_msg, sizeof(result_msg), "SCORE: %d/%d\n", score, num_questions);
    send(client_socket, result_msg, strlen(result_msg), 0);
}
void start_practice_mode(int num_questions_total, int time_limit, int num_easy, int num_medium, int num_hard, const char* subjects, int client_socket) {
    Question easy_questions[MAX_QUESTIONS], medium_questions[MAX_QUESTIONS], hard_questions[MAX_QUESTIONS];
    int easy_count, medium_count, hard_count;
    load_questions_practices();
    printf("%d %d %d %d %d %s\n", num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects);

    // Lọc câu hỏi theo độ khó và môn học
    filter_questions(easy_questions, &easy_count, 1, subjects, client_socket);
    filter_questions(medium_questions, &medium_count, 2, subjects, client_socket);
    filter_questions(hard_questions, &hard_count, 3, subjects, client_socket);

    if (easy_count < num_easy || medium_count < num_medium || hard_count < num_hard) {
        const char *error_msg = "Not enough questions available for the selected criteria.\n";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return;
    }

    srand(time(NULL));

    // Chọn câu hỏi ngẫu nhiên theo độ khó
    Question selected_questions[num_questions_total];
    int index = 0;
    for (int i = 0; i < num_easy; i++) {
        selected_questions[index++] = easy_questions[rand() % easy_count];
    }
    for (int i = 0; i < num_medium; i++) {
        selected_questions[index++] = medium_questions[rand() % medium_count];
    }
    for (int i = 0; i < num_hard; i++) {
        selected_questions[index++] = hard_questions[rand() % hard_count];
    }
    for(int i = 0; i < num_questions_total; i++){
        printf("%s\n", selected_questions[i].question);
    }
    // Cập nhật thời gian bắt đầu cho client
    client_data[client_socket].start_time = time(NULL);

    int score = 0;
    time_t start_time = client_data[client_socket].start_time;
    time_limit *= 60;  // Chuyển đổi phút sang giây
    char client_answers[num_questions_total];
    int flag_time = 0;
    memset(client_answers, 0, sizeof(client_answers));

    // Quản lý thời gian và câu trả lời cho từng client
for (int i = 0; i < num_questions_total; i++) {
    // Tính toán time_left trước
    int time_left = time_limit - difftime(time(NULL), start_time);
    
    if (time_left <= 0) {
        score = calculate_score_practice(client_answers, selected_questions, num_questions_total, client_socket);
        char timeout[BUFFER_SIZE];
        snprintf(timeout, sizeof(timeout), "TIMEOUT: SCORE: %d/%d\n", score, num_questions);
        send(client_socket, timeout, strlen(timeout), 0);
        return;
    }
    
    printf("Time left1: %d seconds\n", time_left);
    // Gửi câu hỏi đến client
    Question* q = &selected_questions[i];
    char question_msg[BUFFER_SIZE];
    snprintf(question_msg, sizeof(question_msg),
            "Question %d/%d:\n%s\n A. %s\n B. %s\n C. %s\n D. %s\n",
            i + 1, num_questions_total, q->question, q->option_A, q->option_B, q->option_C, q->option_D);
    send(client_socket, question_msg, strlen(question_msg), 0);

    // Vòng lặp nhận câu trả lời từ client và cập nhật thời gian liên tục
    while (1) {
        // Cập nhật lại time_left mỗi vòng lặp
        time_left = time_limit - difftime(time(NULL), start_time);
        printf("Time left2: %d seconds\n", time_left);

        // Kiểm tra thời gian ngay lập tức
        if (time_left <= 0) {
            score = calculate_score_practice(client_answers, selected_questions, num_questions_total, client_socket);
            char timeout[BUFFER_SIZE];
            snprintf(timeout, sizeof(timeout), "TIMEOUT: SCORE: %d/%d\n", score, num_questions_total);
            send(client_socket, timeout, strlen(timeout), 0);
            return; // Thoát ngay lập tức khi hết thời gian
        }

        // Nhận dữ liệu từ client
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            const char *disconnect_msg = "Client disconnected.\n";
            send(client_socket, disconnect_msg, strlen(disconnect_msg), 0);
            return;
        }

        buffer[bytes_received] = '\0';

        // Kiểm tra yêu cầu thời gian còn lại từ client
        if (strncmp(buffer, "TIME", 4) == 0) {
            // Nếu client yêu cầu thời gian còn lại, gửi lại thời gian
            time_left = time_limit - difftime(time(NULL), start_time);
            if (time_left <= 0) {
                score = calculate_score_practice(client_answers, selected_questions, num_questions_total, client_socket);
                char timeout[BUFFER_SIZE];
                snprintf(timeout, sizeof(timeout), "TIMEOUT: SCORE: %d/%d\n", score, num_questions_total);
                send(client_socket, timeout, strlen(timeout), 0);
                return;
            } else {
                char time_msg[BUFFER_SIZE];
                snprintf(time_msg, sizeof(time_msg), "Time left: %d seconds\n", time_left);
                printf("Time left3: %d seconds\n", time_left);
                send(client_socket, time_msg, strlen(time_msg), 0);
                continue;  // Tiếp tục vòng lặp nếu yêu cầu thời gian
            }
        } else if (strcmp(buffer, "SUBMIT") == 0) {
            // Nhận yêu cầu submit
            score = calculate_score_practice(client_answers, selected_questions, num_questions_total, client_socket);
            send_result_to_client(score, num_questions_total, client_socket);
            return;
        } else if (strlen(buffer) == 1 && (buffer[0] == 'A' || buffer[0] == 'B' || buffer[0] == 'C' || buffer[0] == 'D')) {
            // Lưu câu trả lời của client
            client_answers[i] = buffer[0];
            time_left = time_limit - difftime(time(NULL), start_time);
            printf("Time left4: %d seconds\n", time_left);
            if (time_left <= 0) {
                score = calculate_score_practice(client_answers, selected_questions, num_questions_total, client_socket);
                char timeout[BUFFER_SIZE];
                snprintf(timeout, sizeof(timeout), "TIMEOUT: SCOREX: %d/%d\n", score, num_questions_total);
                send(client_socket, timeout, strlen(timeout), 0);
                return;
            }
            break;  // Câu trả lời hợp lệ, thoát vòng lặp
        }
    }
}


    score = calculate_score_practice(client_answers, selected_questions, num_questions_total, client_socket);
    send_result_to_client(score, num_questions_total, client_socket);
}

