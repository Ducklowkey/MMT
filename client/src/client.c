// client/src/client.c
#include "../include/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

int connect_to_server(const char* address, int port) {
    int sock;
    struct sockaddr_in server_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return -1;
    }

    printf("\033[1;32mĐang kết nối tới server \033[0m\n");
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("\033[1;32mKết nối server thành công \033[0m\n");
    return sock;
}

void disconnect_from_server(Client* client) {
    if (client->socket >= 0) {
        close(client->socket);
        client->socket = -1;
    }
}

int send_message(Client* client, const char* message) {
    ssize_t sent = send(client->socket, message, strlen(message), 0);
    if (sent < 0) {
        perror("Send failed");
        return -1;
    }
    return sent;
}

int receive_message(Client* client, char* buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (received < 0) {
        perror("Receive failed");
        return -1;
    }
    buffer[received] = '\0';
    return received;
}

void show_auth_menu() {
    printf("\n\033[1;34m===Đăng nhập - Đăng ký===\033[0m\n");
    printf("1. Đăng ký\n");
    printf("2. Đăng nhập\n");
    printf("3. Thoát\n");
}

int handle_authentication(Client* client) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    int choice;
    char input[256];

    while (1) {
        show_auth_menu();
        
        // Đọc input dạng chuỗi
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\033[1;31mLỗi khi đọc input\033[0m\n");
            continue;
        }

        // Xóa newline
        input[strcspn(input, "\n")] = 0;

        // Kiểm tra input chỉ là 1 ký tự và là 1, 2 hoặc 3
        if (strlen(input) != 1 || input[0] < '1' || input[0] > '3') {
            printf("\033[1;31mLựa chọn không hợp lệ. Hãy chọn (1-3)\033[0m\n");
            continue;
        }

        choice = input[0] - '0';

        if (choice == 3) {
            printf("\033[1;34mTiến hành đăng xuất...\033[0m\n");
            return 0;
        }

        printf("\033[1;34mTài khoản: \033[0m");
        if (!fgets(username, sizeof(username), stdin)) continue;
        username[strcspn(username, "\n")] = 0;

        printf("\033[1;34mMật khẩu: \033[0m");
        if (!fgets(password, sizeof(password), stdin)) continue;
        password[strcspn(password, "\n")] = 0;

        // Create command
        memset(command, 0, BUFFER_SIZE);
        snprintf(command, BUFFER_SIZE, "%s %s %s", 
                choice == 1 ? "REGISTER" : "LOGIN", username, password);

        if (send_message(client, command) < 0) {
            printf("\033[1;31mGửi thông điệp thất bại\033[0m\n");
            continue;
        }

        printf("\033[1;34mChờ phản hồi từ server...\033[0m\n");
        memset(buffer, 0, BUFFER_SIZE);
        int received = receive_message(client, buffer);
        if (received <= 0) {
            printf("\033[1;31mServer không phản hồi\033[0m\n");
            continue;
        }

        // If login successful
        if (choice == 2 && strstr(buffer, "success") != NULL) {
            client->is_authenticated = 1;
            strncpy(client->username, username, MAX_USERNAME - 1);
            printf("\n\033[1;32mĐăng nhập thành công!\033[0m\n");
            return 1;
        }
        // If registration successful
        else if (choice == 1 && strstr(buffer, "success") != NULL) {
            printf("\033[1;32mĐăng ký thành công!\033[0m\n");
        }
        else {
            printf("\033[1;31mTài khoản không tồn tại, hoặc sai mật khẩu. Hãy thử lại!\033[0m\n");
        }
    }
}

int authenticate(Client* client, const char* username, const char* password, int is_register) {
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Prepare authentication command
    snprintf(command, BUFFER_SIZE, "%s %s %s",
             is_register ? "REGISTER" : "LOGIN",
             username, password);

    printf("Sending auth command: %s\n", command);

    // Send authentication request
    if (send_message(client, command) < 0) {
        return -1;
    }

    // Receive response
    memset(buffer, 0, BUFFER_SIZE);
    int received = receive_message(client, buffer);
    if (received <= 0) {
        return -1;
    }

    printf("Auth response: %s\n", buffer);

    // Check response
    if (strstr(buffer, "success") != NULL) {
        strncpy(client->username, username, MAX_USERNAME - 1);
        client->is_authenticated = 1;
        return 0;
    }

    return -1;
}
