// server/src/auth.c
#include "../include/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void hash_password(const char* password, char* hashed) {
    // Simple hash function for demonstration
    // In production, use a proper cryptographic hash function
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    snprintf(hashed, 50, "%lu", hash);
}

void handle_authentication(ClientInfo* client, const char* command) {
    char response[BUFFER_SIZE];
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    ssize_t sent;

    printf("Auth command received: %s\n", command);

    // Parse command
    if (sscanf(command, "%*s %s %s", username, password) != 2) {
        printf("Invalid auth command format\n");
        snprintf(response, BUFFER_SIZE, "Invalid command format\n");
        sent = send(client->fd, response, strlen(response), 0);
        if (sent < 0) perror("Send failed");
        return;
    }

    printf("Processing auth for user: %s\n", username);

    if (strncmp(command, "REGISTER", 8) == 0) {
        int result = register_user(username, password);
        if (result == 0) {
            printf("Registration successful for: %s\n", username);
            snprintf(response, BUFFER_SIZE, "Registration successful!\n");
        } else if (result == -2) {
            printf("Registration failed - username exists: %s\n", username);
            snprintf(response, BUFFER_SIZE, "Username already exists\n");
        } else {
            printf("Registration failed for: %s\n", username);
            snprintf(response, BUFFER_SIZE, "Registration failed\n");
        }
    }
    else if (strncmp(command, "LOGIN", 5) == 0) {
        char session_id[50];
        int result = login_user(username, password, session_id);
        if (result == 0) {
            printf("Login successful for: %s\n", username);
            strncpy(client->username, username, MAX_USERNAME - 1);
            strncpy(client->session_id, session_id, sizeof(client->session_id) - 1);
            client->authenticated = 1;
            client->session_start = time(NULL);
            snprintf(response, BUFFER_SIZE, "Login successful!\n");
        } else {
            printf("Login failed for: %s\n", username);
            snprintf(response, BUFFER_SIZE, "Login failed\n");
        }
    }

    printf("Sending auth response: %s", response);
    sent = send(client->fd, response, strlen(response), 0);
    if (sent < 0) {
        perror("Send failed");
    } else {
        printf("Auth response sent successfully\n");
    }
}

int register_user(const char* username, const char* password) {
    printf("Attempting to register user: %s\n", username);

    FILE* file = fopen("users.txt", "a+");
    if (!file) {
        perror("Cannot open users file");
        return -1;
    }

    // Check if username exists
    char line[BUFFER_SIZE];
    rewind(file);
    while (fgets(line, BUFFER_SIZE, file)) {
        char stored_username[MAX_USERNAME];
        if (sscanf(line, "%s", stored_username) == 1) {
            if (strcmp(stored_username, username) == 0) {
                fclose(file);
                return -2; // Username exists
            }
        }
    }

    // Add new user
    char hashed_password[50];
    hash_password(password, hashed_password);
    fprintf(file, "%s %s\n", username, hashed_password);
    fflush(file); // Ensure data is written
    fclose(file);

    printf("User registered successfully: %s\n", username);
    return 0;
}

int login_user(const char* username, const char* password, char* session_id) {
    printf("Attempting login for user: %s\n", username);

    FILE* file = fopen("users.txt", "r");
    if (!file) {
        perror("Cannot open users file");
        return -1;
    }

    char line[BUFFER_SIZE];
    char hashed_password[50];
    hash_password(password, hashed_password);

    while (fgets(line, BUFFER_SIZE, file)) {
        char stored_username[MAX_USERNAME];
        char stored_password[50];
        if (sscanf(line, "%s %s", stored_username, stored_password) == 2) {
            if (strcmp(stored_username, username) == 0 &&
                strcmp(stored_password, hashed_password) == 0) {
                // Generate session ID
                snprintf(session_id, 50, "%d%ld", rand(), time(NULL));
                fclose(file);
                printf("Login successful for user: %s\n", username);
                return 0;
            }
        }
    }

    printf("Login failed for user: %s\n", username);
    fclose(file);
    return -1;
}

void handle_logout(ClientInfo* client) {
    if (client->current_room_id != -1) {
        leave_exam_room(client->current_room_id, client->username, client);
    }

    memset(client->username, 0, MAX_USERNAME);
    memset(client->session_id, 0, sizeof(client->session_id));
    client->authenticated = 0;
    client->session_start = 0;
    client->current_room_id = -1;

    char response[] = "Logged out successfully\n";
    send(client->fd, response, strlen(response), 0);
}
