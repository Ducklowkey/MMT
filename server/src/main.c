// server/src/main.c
#include "../include/server.h"
#include "../include/exam.h"
#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

volatile sig_atomic_t server_running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived shutdown signal. Cleaning up...\n");
        server_running = 0;
    }
}

int main() {
    // Set up signal handlers
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up signal handler");
        return EXIT_FAILURE;
    }

    // Initialize server
    Server* server = create_server();
    if (!server) {
        fprintf(stderr, "Failed to create server\n");
        return EXIT_FAILURE;
    }

    // Initialize database
    init_database();

    // Load exam questions
    load_questions();

    printf("Server starting on port %d...\n", PORT);

    server->running = 1;
    start_server(server);

    // Cleanup
    printf("Shutting down server...\n");
    destroy_server(server);
    close_database();

    printf("Server shutdown complete\n");
    return EXIT_SUCCESS;
}
