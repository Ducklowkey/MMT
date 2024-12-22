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

    printf("Connecting to server...\n");
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to server successfully!\n");
    return sock;
}

void disconnect_from_server(Client* client) {
    if (client->socket >= 0) {
        close(client->socket);
        client->socket = -1;
    }
}

int send_message(Client* client, const char* message) {
    printf("Sending message: %s\n", message);
    ssize_t sent = send(client->socket, message, strlen(message), 0);
    if (sent < 0) {
        perror("Send failed");
        return -1;
    }
    printf("Sent %zd bytes\n", sent);
    return sent;
}

int receive_message(Client* client, char* buffer) {
    printf("Waiting for response...\n");
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (received < 0) {
        perror("Receive failed");
        return -1;
    }
    buffer[received] = '\0';
    printf("Received %zd bytes: %s\n", received, buffer);
    return received;
}

void show_auth_menu() {
    printf("\n=== Authentication ===\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. Exit\n");
    printf("Choose an option (1-3): ");
}

int handle_authentication(Client* client) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    int choice;

    while (1) {
        show_auth_menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input\n");
            continue;
        }
        while (getchar() != '\n');

        if (choice == 3) {
            printf("Exiting...\n");
            return 0;
        }

        printf("Enter username: ");
        if (!fgets(username, sizeof(username), stdin)) continue;
        username[strcspn(username, "\n")] = 0;

        printf("Enter password: ");
        if (!fgets(password, sizeof(password), stdin)) continue;
        password[strcspn(password, "\n")] = 0;

        // Create command
        memset(command, 0, BUFFER_SIZE);
        snprintf(command, BUFFER_SIZE, "%s %s %s",
                choice == 1 ? "REGISTER" : "LOGIN",
                username, password);

        printf("Sending command: %s\n", command);
        if (send_message(client, command) < 0) {
            printf("Failed to send command\n");
            continue;
        }

        printf("Waiting for server response...\n");
        memset(buffer, 0, BUFFER_SIZE);
        int received = receive_message(client, buffer);
        if (received <= 0) {
            printf("No response from server\n");
            continue;
        }

        printf("Server response: %s", buffer);

        // If login successful
        if (choice == 2 && strstr(buffer, "success") != NULL) {
            client->is_authenticated = 1;
            strncpy(client->username, username, MAX_USERNAME - 1);
            printf("Login successful!\n");
            return 1;
        }
        // If registration successful
        else if (choice == 1 && strstr(buffer, "success") != NULL) {
            printf("Registration successful! Please login.\n");
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
