#include "../include/exam.h"
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <ctype.h>

void handle_exam(Client* client) {
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int max_fd = client->socket;
    int exam_completed = 0;
    char current_answer = '\0';  // Lưu đáp án hiện tại

    while (!exam_completed) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client->socket, &readfds);

        struct timeval tv;
        tv.tv_sec = 1;  // Timeout 1 giây
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0) {
            print_error("Select error");
            break;
        }

        // Kiểm tra tin nhắn từ server
        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);
            if (valread <= 0) {
                print_error("Server disconnected");
                break;
            }

            // Xử lý tin nhắn
            if (strstr(buffer, "Question") != NULL) {
                clear_screen();  // Xóa màn hình cũ
                printf("\n%s", buffer);
                printf("\nYour answer (A/B/C/D): ");
                fflush(stdout);
                current_answer = '\0';  // Reset đáp án
            }
            else if (strstr(buffer, "Exam completed") != NULL) {
                printf("\n%s", buffer);
                exam_completed = 1;
                break;
            }
            else {
                printf("%s", buffer);
            }
        }

        // Kiểm tra input từ người dùng
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char input;
            if (read(STDIN_FILENO, &input, 1) > 0) {
                if (input >= 'a' && input <= 'd') input -= 32;  // Chuyển sang chữ hoa
                if (input >= 'A' && input <= 'D') {
                    current_answer = input;
                    char cmd[BUFFER_SIZE];
                    snprintf(cmd, BUFFER_SIZE, "SUBMIT_ANSWER %c", current_answer);
                    if (send_message(client, cmd) < 0) {
                        print_error("Failed to send answer");
                    } else {
                        printf("\nSubmitted answer: %c\n", current_answer);
                    }
                }
            }
        }
    }
}
