#include "../include/practice.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // Để dùng read và write
#include <sys/socket.h>  // Dành cho Linux/Mac
#include <arpa/inet.h>   // Dành cho Linux/Mac

int subject_count = 0;
char subject_name[MAX_SUBJECTS][BUFFER_SIZE] = {0};  // 2D array to store subject names
// Hàm cho phép người dùng lựa chọn thông số bài luyện tập
void configure_practice(Client* client, int* num_questions_total, int* time_limit, int* num_easy, int* num_medium, int* num_hard, char* subjects) {
    
    // printf("Select subjects:\n");
    // printf("1. Math\n2. Geography\n3. History\n4. Literature\n5. English\n6. Physics\n7. Chemistry\n8. Biology\n9. All\n");

     int selected_subjects[MAX_SUBJECTS] = {0};
     int subject_choice;
     int dem = 0;
     int result=0; // check scanf 
     char subjects_practice[BUFFER_SIZE];  // Chuỗi lưu các môn học đã chọn
    memset(subjects_practice, 0, sizeof(subjects_practice));
    memset(subjects, 0, sizeof(subjects));
    print_subjects_menu(client);
    while (1) {
        printf("Subject number (or 0 to finish): ");
        //scanf("%d", &subject_choice);
        result = scanf("%d", &subject_choice);
        if (result != 1) {
            while (getchar() != '\n');  // Loại bỏ ký tự còn lại trong bộ đệm
            continue;
        } 
        if (subject_choice == 0 && dem > 0) {
            break;
        } else if (subject_choice >= 1 && subject_choice <= subject_count) {
            if (!selected_subjects[subject_choice - 1]) {
                selected_subjects[subject_choice - 1] = 1;
                dem++;
                printf("Subject %s selected.\n", subject_name[subject_choice - 1]);
            } else {
                printf("Subject already selected.\n");
            }
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }

    // Tạo chuỗi các môn học đã chọn
    subjects_practice[0] = '\0';  // Đặt chuỗi subjects ban đầu trống
    for (int i = 0; i < subject_count; i++) {
        if (selected_subjects[i]) {
            strcat(subjects, subject_name[i]);
            strcat(subjects, ",");
        }
    }
    // Loại bỏ dấu phẩy thừa ở cuối chuỗi
    if (strlen(subjects_practice) > 0) {
        subjects[strlen(subjects_practice) - 1] = '\0';  // Loại bỏ dấu phẩy cuối
    }
    printf("Selected subjects: %s\n", subjects_practice);

    printf("Select total number of questions:\n");
    printf("1. 15\n2. 30\n3. 45\n4. 60\n");
    int question_choice=2;
    result=0;
    while (1){
        printf("Enter your choice: ");
        //scanf("%d", &question_choice);
        result = scanf("%d", &question_choice);
        if (result != 1) {
            while (getchar() != '\n');  // Loại bỏ ký tự còn lại trong bộ đệm
            continue;
        } 
        if (question_choice < 1 || question_choice > 4) continue;
        switch (question_choice) {
            case 1: *num_questions_total = 15; break;
            case 2: *num_questions_total = 30; break;
            case 3: *num_questions_total = 45; break;
            case 4: *num_questions_total = 60; break;
            default: printf("Invalid choice. Try again.\n");
        }
        break;
    }
    printf("Select difficulty ratio (Easy:Medium:Hard):\n");
    printf("1. 3:0:0\n2. 0:3:0\n3. 0:0:3\n4. 1:2:0\n5. 2:1:0\n6. 1:0:2\n7. 2:0:1\n8. 0:1:2\n9. 0:2:1\n10. 1:1:1\n");
    int ratio_choice=0;
    result=0;
    while (1) {
        printf("Enter your choice: ");
        //scanf("%d", &ratio_choice);
        int result = scanf("%d", &ratio_choice);
        if (result != 1)  {while (getchar() != '\n'); }
        else{
            if (ratio_choice <1 || ratio_choice > 10) {
                continue;
            }
            else{
            switch (ratio_choice) {
                case 1: *num_easy = *num_questions_total; *num_medium = 0; *num_hard = 0; break;
                case 2: *num_easy = 0; *num_medium = *num_questions_total; *num_hard = 0; break;
                case 3: *num_easy = 0; *num_medium = 0; *num_hard = *num_questions_total; break;
                case 4: *num_easy = *num_questions_total / 3; *num_medium = *num_questions_total * 2 / 3; *num_hard = 0; break;
                case 5: *num_easy = *num_questions_total * 2 / 3; *num_medium = *num_questions_total / 3; *num_hard = 0; break;
                case 6: *num_easy = *num_questions_total / 3; *num_medium = 0; *num_hard = *num_questions_total * 2 / 3; break;
                case 7: *num_easy = *num_questions_total * 2 / 3; *num_medium = 0; *num_hard = *num_questions_total / 3; break;
                case 8: *num_easy = 0; *num_medium = *num_questions_total / 3; *num_hard = *num_questions_total * 2 / 3; break;
                case 9: *num_easy = 0; *num_medium = *num_questions_total * 2 / 3; *num_hard = *num_questions_total / 3; break;
                case 10: *num_easy = *num_questions_total / 3; *num_medium = *num_questions_total / 3; *num_hard = *num_questions_total / 3; break;
                //default: printf("Invalid choice. Try again.\n");
            }
            break;
            }
        }
    }
    //printf("Enter time limit (in minutes): ");
    //scanf("%d", time_limit);
    result=0;
    while (1) {
        printf("Enter time limit (in minutes): ");
        int result = scanf("%d", time_limit);
        if (result == 1) {
            if (*time_limit > 0 && *time_limit <= 120) {
                break;
            } else {
                printf("Invalid time limit. Please enter a value between 1 and 120 minutes.\n");
            }
        } else {
            while (getchar() != '\n');  // Loại bỏ ký tự còn lại trong bộ đệm
        }
    }
}
// Hàm gửi thông số bài luyện tập đến server
void send_practice_config(Client* client, int num_questions_total, int time_limit, int num_easy, int num_medium, int num_hard, char* subjects) {
    // Chuẩn bị thông điệp gửi đi
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer),"START_PRACTICE %d,%d,%d,%d,%d,%s\n",
            num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects); // Chuyển thời gian từ phút sang giây
    //printf("Practice config: %s\n", buffer);
    if (write(client->socket, buffer, strlen(buffer)) > 0) {
        printf("Practice config sent to server: %s\n", buffer);
    } else {
        printf("Failed to send practice config.\n");
    }
    
}

// Hàm send thông điệp START_PRACTICE VÀ FORMAT bài luyện tập cho server
void start_and_set_format(Client* client){
   char buffer[1024];
    char subjects[256];
    int num_questions_total = 0, num_easy = 0, num_medium = 0, num_hard = 0, time_limit = 0;

    // Gọi hàm cấu hình bài luyện tập
    configure_practice(client, &num_questions_total, &time_limit, &num_easy, &num_medium, &num_hard, subjects);
    send_practice_config(client, num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects); 
}
void handle_practice(Client* client) {
    char buffer[1024];
    while (1) {
        // Nhận câu hỏi từ server
        memset(buffer, 0, sizeof(buffer)); // Xóa nội dung cũ của buffer
        int bytes_received = read(client->socket, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0) {
            printf("Disconnected from server.\n");
            break;
        }
        if( buffer[bytes_received] != '\0')
            buffer[bytes_received] = 0; // Đảm bảo buffer là chuỗi kết thúc null
        // Client kiểm tra tín hiệu TIMEOUT
        if (strncmp(buffer, "TIMEOUT:", 8) == 0) {
            printf("%s\n", buffer);  // In thông báo hết thời gian và điểm số
            return;
        }
        if (strncmp(buffer, "SCORE:", 6) == 0) {
            printf("%s\n", buffer);
            return;
        }
        if (strncmp(buffer, "ERROR_FORMAT", 12) == 0) {
            printf("%s\n", buffer);
            //start_and_set_format(client);
            return;
        }
       // printf("%s\n", buffer);
        if (strncmp(buffer, "CHANGE_SUCCESS", 14) == 0 || strncmp(buffer, "ERROR_CHANGE_ANS", 16) == 0) {
            printf("%s\n", buffer);
            printf("Continue answering the question:");
        }
        else 
        {
            printf("%s\n", buffer);
            printf("Enter your answer: \n  A/B/C/D \n  < SUBMIT > to quit \n  < TIME > to request time left\n  < CHANGE > to change answer\nAnswer: ");
        }
        char answer[256];
        while (1) {
            scanf("%s", answer);
            if (strcmp(answer, "SUBMIT") == 0 || strcmp(answer, "TIME") == 0 || strcmp(answer,"CHANGE")==0 || (strlen(answer) == 1 &&
                (answer[0] == 'A' || answer[0] == 'B' || answer[0] == 'C' || answer[0] == 'D'))) {
                break;
            }
            else printf("Enter your answer:");// \n  A/B/C/D \n  < SUBMIT > to quit \n  < TIME > to request time left\n  < CHANGE > to change answer\nAnswer: ");
        }
       // printf("answer: %s\n", answer);
        if(strcmp(answer,"CHANGE") == 0){
           // printf("Enter question number: ");
            change_answer(client);
            //continue;
        }
        else
            submit_answer_practice(client, answer);
    }
}
void change_answer(Client* client){
    int question_number;
    char new_answer[2];
    printf("Enter question number ");
    scanf("%d", &question_number);
    printf("Enter new answer ");
    scanf("%s", new_answer);
    while(1)
    { 
        if(strlen(new_answer) == 1 &&
                (new_answer[0] == 'A' || new_answer[0] == 'B' || new_answer[0] == 'C' || new_answer[0] == 'D')){
            break;
            }
           // printf("Change answer: %d %s\n", question_number, new_answer);}
        else 
        { 
            printf("Invalid answer. Please enter A/B/C/D: ");
        }   scanf("%s", new_answer);
    }
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "CHANGE_ANSWER_PRACTICE %d %c\n", question_number, new_answer[0]);
    if (write(client->socket, buffer, strlen(buffer)) > 0) {
        printf("Change answer sent to server: %s\n", buffer);
    } else {
        printf("Failed to send change answer.\n");
    }
}
// Hàm gửi câu trả lời thực hành
void submit_answer_practice(Client* client, const char* answer) {
    // Kiểm tra tính hợp lệ của client và socket
    if (client == NULL || answer == NULL) {
        fprintf(stderr, "Client hoặc câu trả lời không hợp lệ.\n");
        return;
    }
    const char* command = "SUBMIT_PRACTICE_ANSWER"; // Lệnh gửi câu trả lời
    char message[256]; // Kích thước đủ lớn để chứa thông điệp

    // Tạo thông điệp gửi
    snprintf(message, sizeof(message), "%s %s\n", command, answer);

    // Gửi thông điệp qua socket
    if (write(client->socket, message, strlen(message)) > 0) {
        printf("Submit answer practice success.\n");
    } else {
        perror("Submit answer practice failed"); // In lỗi chi tiết
    }
}

void print_subjects_menu(Client* client) {
    char buffer[BUFFER_SIZE];
    char command[] = "GET_SUBJECTS";
    // Send request to get the list of subjects
    if (send_message(client, command) < 0) {
        printf("Error sending request to get subjects list\n");
        return;
    }

    // Receive the list of subjects
    if (receive_message(client, buffer) <= 0) {
        printf("Error receiving data from the server\n");
        return;
    }
    printf("%s", buffer);

    buffer[strlen(buffer) - 1] = '\0';  // Remove the newline character at the end
    if (strncmp(buffer, "SUBJECTS|", 9) == 0) {
        char* subjects_str = buffer + 9;
        char* subject;
        int index = 1;

        printf("Select subjects:\n");

        // Split and store each subject into the subject_name array
        memset(subject_name, 0, sizeof(subject_name));
        subject_count = 0;  // Reset the subject count
        subject = strtok(subjects_str, ",");
        while (subject != NULL) {
            // Store the subject in the subject_name array
            if (subject_count < MAX_SUBJECTS) {
                strncpy(subject_name[subject_count], subject, BUFFER_SIZE - 1);
                subject_name[subject_count][BUFFER_SIZE - 1] = '\0';  // Ensure null termination
                //printf("%d. %s\n", index++, subject_name[subject_count]);
                subject_count++;
            } else {
                printf("Error: Number of subjects exceeds the limit.\n");
                break;
            }
            subject = strtok(NULL, ",");
        }
            strncpy(subject_name[subject_count], "All", BUFFER_SIZE - 1);
            subject_count++;
            //printf("%d. %s\n", subject_count+1, subject_name[subject_count]);
            for(int i = 0; i < subject_count; i++){
                printf("%d. %s\n", i+1, subject_name[i]);
            } 
    } else {
        printf("Invalid data received.\n");
    }
}