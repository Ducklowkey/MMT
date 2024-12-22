// server/src/exam.c
#include "../include/exam.h"
#include "../include/server.h"

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

    // Print debug info
    printf("Sending question %d to client %d\n", question_number + 1, client->fd);

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

void handle_answer(ClientInfo* client, char answer) {
    ExamRoom* room = get_room(client->current_room_id);
    if (!room) {
        printf("Error: Room not found for client %d\n", client->fd);
        return;
    }

    printf("Processing answer '%c' from client %d (question %d)\n", 
           answer, client->fd, client->current_question + 1);

    char buffer[BUFFER_SIZE];
    
    // Kiểm tra đáp án
    if (answer == questions[room->question_ids[client->current_question]].correct_answer) {
        client->score++;
        snprintf(buffer, BUFFER_SIZE, "\nCorrect answer! Your current score: %d/%d\n",
                client->score, room->num_questions);
    } else {
        snprintf(buffer, BUFFER_SIZE, "\nWrong answer! The correct answer was %c\n",
                questions[room->question_ids[client->current_question]].correct_answer);
    }

    // Gửi kết quả câu trả lời
    if (send(client->fd, buffer, strlen(buffer), 0) < 0) {
        printf("Error sending answer result to client %d\n", client->fd);
        return;
    }

    // Chuyển sang câu hỏi tiếp theo
    client->current_question++;

    printf("Client %d moving to question %d/%d\n", 
           client->fd, client->current_question + 1, room->num_questions);

    // Nếu còn câu hỏi thì gửi câu tiếp theo
    if (client->current_question < room->num_questions) {
        // Đợi một chút trước khi gửi câu hỏi tiếp theo
        usleep(100000);  // 100ms delay
        send_question(client, client->current_question);
    } else {
        // Kết thúc bài thi
        snprintf(buffer, BUFFER_SIZE, "\nExam completed! Your final score: %d/%d\n",
                client->score, room->num_questions);
        send(client->fd, buffer, strlen(buffer), 0);
        
        // Lưu kết quả
        ExamResult result = {
            .exam_id = room->room_id,
            .score = client->score,
            .total_questions = room->num_questions,
            .exam_date = time(NULL)
        };
        strncpy(result.username, client->username, MAX_USERNAME - 1);
        save_exam_result_to_db(&result);

        printf("Client %d completed exam with score %d/%d\n", 
               client->fd, client->score, room->num_questions);

        // Reset trạng thái
        client->current_question = -1;
        client->score = 0;
    }
}

void start_exam(Server* server, ExamRoom* room) {
    printf("Starting exam in room %d\n", room->room_id);
    room->status = 1;

    // Khởi tạo exam session
    room->num_questions = NUM_QUESTIONS_PER_EXAM;
    
    // Random chọn câu hỏi
    srand(time(NULL));
    for(int i = 0; i < room->num_questions; i++) {
        room->question_ids[i] = rand() % num_questions;
        printf("Selected question %d for position %d\n", room->question_ids[i], i + 1);
    }
    
    // Gửi thông báo và câu hỏi đầu tiên cho mọi người
    for (int i = 0; i < room->user_count; i++) {
        for (int j = 0; j < MAX_CLIENTS; j++) {
            if (server->clients[j].active && 
                strcmp(server->clients[j].username, room->users[i]) == 0) {
                
                // Khởi tạo trạng thái client
                server->clients[j].current_question = 0;
                server->clients[j].score = 0;

                // Gửi thông báo bắt đầu
                char msg[BUFFER_SIZE];
                snprintf(msg, BUFFER_SIZE, "Exam has started! Total questions: %d\n", 
                        room->num_questions);
                
                printf("Starting exam for client %d (%s)\n", 
                       server->clients[j].fd, server->clients[j].username);

                send(server->clients[j].fd, msg, strlen(msg), 0);
                
                // Đợi một chút rồi gửi câu hỏi đầu tiên
                usleep(100000);  // 100ms delay
                send_question(&server->clients[j], 0);
                break;
            }
        }
    }
}