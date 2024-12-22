#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Hàm in ra banner chào mừng người dùng khi chương trình bắt đầu
void print_banner(void) {
    printf("\n===================================\n");  // In ra dòng phân cách trên cùng
    printf("Welcome to the Online Exam System\n"); // Chào mừng người dùng
    printf("===================================\n\n"); // In ra dòng phân cách dưới cùng
}

// Hàm in ra menu chính của chương trình, người dùng có thể chọn các chức năng
// như tạo phòng thi, tham gia phòng thi hoặc đăng xuất
void print_main_menu(void) {
    printf("\n=== Main Menu ===\n");
    printf("1. Create Exam Room\n");  // Tạo phòng thi mới
    printf("2. Join Exam Room\n");    // Tham gia phòng thi hiện có
    printf("3. Practice Room\n");            // Đăng xuất khỏi hệ thống
    printf("4. Logout\n");           // Đăng xuất khỏi hệ thống
}
void print_practice_room_menu(void) {
    printf("\n=== Practice Mode Menu ===\n");
    printf("1. Start Practice\n");      // Tùy chọn bắt đầu thi
    printf("2. Leave Practice Mode\n");      // Tùy chọn rời phòng
}
// Hàm in ra menu phòng thi, tùy thuộc vào người dùng có phải là chủ phòng thi hay không
// Chủ phòng có thêm các lựa chọn như bắt đầu thi, xóa phòng, rời phòng
// Người tham gia phòng chỉ có thể rời phòng và chờ bắt đầu thi
void print_room_menu(int is_creator) {
    printf("\n=== Exam Room Menu ===\n");
    if (is_creator) {  // Nếu người dùng là chủ phòng
        printf("1. Start Exam\n");      // Tùy chọn bắt đầu thi
        printf("2. Leave Room\n");      // Tùy chọn rời phòng
        printf("3. Delete Room\n");     // Tùy chọn xóa phòng
    } else {  // Nếu người dùng không phải là chủ phòng
        printf("1. Leave Room\n");      // Tùy chọn rời phòng
        printf("\nWaiting for exam to start...\n");  // Thông báo chờ bắt đầu thi
    }
    printf("\nChoose an option: ");  // Yêu cầu người dùng nhập lựa chọn
}
void handle_practice_menu(Client* client) {
    char buffer[BUFFER_SIZE];  // Bộ đệm để lưu dữ liệu nhận từ server
    fd_set readfds;            // Bộ file descriptor để kiểm tra đầu vào
    int max_fd = client->socket;  // File descriptor lớn nhất (socket)

    while (1) {
        print_practice_room_menu();
        // Thiết lập bộ readfds
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);  // Kiểm tra đầu vào từ người dùng
        FD_SET(client->socket, &readfds);  // Kiểm tra dữ liệu từ server

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);  // Đợi sự kiện từ stdin hoặc socket
        if (activity < 0) {
            print_error("Select error");
            break;
        }

        // Kiểm tra nếu có dữ liệu từ server
        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);
            if (valread <= 0) {
                print_error("Server disconnected");
                return;
            }
            printf("\n%s\n", buffer);  // Hiển thị thông báo từ server

            // Kiểm tra nếu server thông báo về sự thay đổi trạng thái
            if (strstr(buffer, "You can now start practicing") != NULL) {
                printf("You can start practicing now.\n");
            }
        }

        // Kiểm tra đầu vào từ người dùng
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int choice;
            if (scanf("%d", &choice) != 1) {  // Kiểm tra đầu vào hợp lệ
                while (getchar() != '\n');  // Loại bỏ ký tự dư thừa
                print_error("Invalid input");
                continue;
            }
            while (getchar() != '\n');  // Loại bỏ ký tự dư thừa

            switch (choice) {
                case 1: {  // Bắt đầu luyện tập
                    if (send_message(client, "START_PRACTICE") >= 0) {
                        // Xử lý tiếp theo nếu có thông báo từ server, ví dụ:
                         handle_practice(client);
                         continue;
                    } else {
                        print_error("Failed to start practice");
                    }
                    break;
                }

                case 2:  // Rời chế độ luyện tập
                    if (send_message(client, "LEAVE_PRACTICE") >= 0) {
                        print_success("Left practice mode");
                        return;  // Quay lại menu chính hoặc thoát
                    } else {
                        print_error("Failed to leave practice mode");
                    }
                    break;

                default:
                    print_error("Invalid option");
                    break;
            }
        }
    }
}

// Hàm để xóa màn hình, tùy thuộc vào hệ điều hành (Windows hay Unix/Linux)
void clear_screen(void) {
#ifdef _WIN32
    system("cls");  // Lệnh xóa màn hình trên Windows
#else
    system("clear");  // Lệnh xóa màn hình trên Unix/Linux
#endif
}

// Hàm in ra thông báo lỗi với màu đỏ
void print_error(const char* message) {
    printf("\033[1;31mError: %s\033[0m\n", message);  // Mã màu đỏ để thông báo lỗi
}

// Hàm in ra thông báo thành công với màu xanh lá cây
void print_success(const char* message) {
    printf("\033[1;32m%s\033[0m\n", message);  // Mã màu xanh lá cây để thông báo thành công
}

// Hàm xử lý menu phòng thi, kiểm tra sự kiện từ máy chủ và người dùng
// Sau khi nhận thông tin từ người dùng hoặc từ máy chủ, thực hiện hành động tương ứng
void handle_room_menu(Client* client) {
    char buffer[BUFFER_SIZE];  // Dùng để chứa dữ liệu nhận từ máy chủ
    fd_set readfds;            // Tập hợp các file descriptor cần kiểm tra (stdin và socket)
    int max_fd = client->socket;  // Lấy giá trị socket cao nhất để truyền vào select

    while (1) {
        print_room_menu(client->is_room_creator);  // In menu phòng thi
        
        FD_ZERO(&readfds);  // Xóa tất cả các file descriptor trong readfds
        FD_SET(STDIN_FILENO, &readfds);  // Thêm stdin vào readfds để kiểm tra đầu vào từ người dùng
        FD_SET(client->socket, &readfds);  // Thêm socket của client vào readfds để kiểm tra sự kiện từ máy chủ

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);  // Chờ sự kiện từ stdin hoặc socket
        if (activity < 0) {  // Nếu có lỗi khi gọi select
            print_error("Select error");
            break;
        }

        // Kiểm tra nếu có dữ liệu từ máy chủ (socket)
        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);  // Nhận dữ liệu từ máy chủ
            if (valread <= 0) {  // Nếu không nhận được dữ liệu hoặc có lỗi
                print_error("Server disconnected");  // In ra thông báo lỗi nếu máy chủ ngắt kết nối
                return;
            }

            // Nếu nhận được thông báo "Exam has started" và người dùng không phải là chủ phòng
            if (strstr(buffer, "Exam has started") != NULL && !client->is_room_creator) {
                printf("\n%s", buffer);  // In thông báo thi đã bắt đầu
                handle_exam(client);  // Chuyển sang màn hình thi
                return;
            }

            printf("\n%s", buffer);  // In thông báo nhận được từ máy chủ
        }

        // Kiểm tra đầu vào từ người dùng nếu không có thông báo từ máy chủ
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int choice;
            if (scanf("%d", &choice) != 1) {  // Kiểm tra đầu vào của người dùng
                while (getchar() != '\n');  // Loại bỏ ký tự dư thừa
                print_error("Invalid input");  // Nếu đầu vào không hợp lệ
                continue;
            }
            while (getchar() != '\n');  // Loại bỏ ký tự dư thừa

            if (client->is_room_creator) {  // Nếu người dùng là chủ phòng
                switch (choice) {
                    case 1:  // Bắt đầu thi
                        if (send_message(client, "START_EXAM") >= 0) {
                            // Chủ phòng không cần xử lý thêm, chỉ cần đợi thông báo từ server
                        }
                        break;

                    case 2:  // Rời phòng
                        if (send_message(client, "LEAVE_ROOM") >= 0) {
                            receive_message(client, buffer);
                            print_success("Left room successfully");  // Thông báo rời phòng thành công
                            return;
                        }
                        break;

                    case 3:  // Xóa phòng
                        if (send_message(client, "DELETE_ROOM") >= 0) {
                            receive_message(client, buffer);
                            print_success("Room deleted successfully");  // Thông báo xóa phòng thành công
                            return;
                        }
                        break;

                    default:  // Nếu lựa chọn không hợp lệ
                        print_error("Invalid option");
                        break;
                }
            } else {  // Nếu người dùng không phải là chủ phòng
                if (choice == 1) {  // Rời phòng
                    if (send_message(client, "LEAVE_ROOM") >= 0) {
                        receive_message(client, buffer);
                        print_success("Left room successfully");  // Thông báo rời phòng thành công
                        return;
                    }
                } else {
                    print_error("Invalid option");  // Nếu lựa chọn không hợp lệ
                }
            }
        }
    }
}

// Hàm xử lý menu chính, nhận đầu vào từ người dùng và thực hiện các hành động tương ứng
// Người dùng có thể tạo phòng thi, tham gia phòng thi, hoặc đăng xuất
void handle_main_menu(Client* client) {
    char buffer[BUFFER_SIZE];  // Dùng để chứa dữ liệu nhận từ máy chủ
    int choice;

    while (1) {
        print_main_menu();  // In menu chính

        // Nhận lựa chọn từ người dùng
        if (scanf("%d", &choice) != 1) {  // Kiểm tra đầu vào của người dùng
            while (getchar() != '\n');  // Loại bỏ ký tự dư thừa
            print_error("Invalid input");  // Nếu đầu vào không hợp lệ
            continue;
        }
        while (getchar() != '\n');  // Loại bỏ ký tự dư thừa

        switch (choice) {
            case 1: {  // Tạo phòng thi
                printf("Enter room name: ");
                char room_name[BUFFER_SIZE];
                char cmd[BUFFER_SIZE];
                
                // Đọc tên phòng từ người dùng
                if (fgets(room_name, BUFFER_SIZE, stdin)) {
                    room_name[strcspn(room_name, "\n")] = 0;  // Loại bỏ ký tự newline sau tên phòng

                    // Kiểm tra độ dài tên phòng
                    if (strlen(room_name) < 1) {
                        print_error("Room name cannot be empty");  // Nếu tên phòng rỗng
                        continue;
                    }

                    // Tạo lệnh với tên phòng
                    memset(cmd, 0, BUFFER_SIZE);
                    if (snprintf(cmd, BUFFER_SIZE, "CREATE_ROOM %s", room_name) >= BUFFER_SIZE) {
                        print_error("Room name too long");  // Nếu tên phòng quá dài
                        continue;
                    }

                    printf("Sending command: %s\n", cmd);

                    // Gửi lệnh tạo phòng đến máy chủ
                    if (send_message(client, cmd) < 0) {
                        print_error("Failed to send command to server");  // Nếu không thể gửi lệnh
                        continue;
                    }

                    // Nhận phản hồi từ máy chủ
                    char response[BUFFER_SIZE];
                    memset(response, 0, BUFFER_SIZE);
                    int received = receive_message(client, response);
                    
                    if (received < 0) {
                        print_error("Failed to receive server response");  // Nếu không nhận được phản hồi từ server
                        continue;
                    }

                    printf("Server response: %s", response);

                    // Xử lý phản hồi từ server
                    if (strstr(response, "success") != NULL) {
                        client->is_room_creator = 1;  // Đánh dấu người dùng là chủ phòng
                        print_success("Room created successfully!");
                        handle_room_menu(client);  // Chuyển sang menu phòng thi
                    } else {
                        print_error(response);  // In thông báo lỗi từ máy chủ
                    }
                } else {
                    print_error("Failed to read room name");  // Nếu không thể đọc tên phòng
                }
                break;
            }

            case 2: {  // Tham gia phòng thi
                if (send_message(client, "LIST_ROOMS") >= 0) {  // Gửi yêu cầu danh sách phòng thi
                    if (receive_message(client, buffer) >= 0) {
                        print_success("List of rooms:");
                        printf("%s\n", buffer);  // In ra danh sách phòng thi
                    }
                }
                break;
            }
            case 3: {  // Chế độ luyện tập
                print_success("Starting practice mode...");
                handle_practice_menu(client);
                break;
            }
            case 4:  // Đăng xuất
                print_success("Logging out...");
                return;  // Thoát khỏi menu chính và quay lại đăng nhập hoặc thoát chương trình

            default:  // Nếu lựa chọn không hợp lệ
                print_error("Invalid option");
                break;
        }
    }
}
