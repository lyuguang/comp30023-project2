#include "task.h"

// Validates input string to ensure all characters are non-control ASCII characters
int validate_input(const char *input) {
    // Loop through each character in the input string
    while (*input) {
        // Check if the character is a control character or not an ASCII character
        if (iscntrl((unsigned char)*input) || (unsigned char)*input > 127) {
            return 0; // Invalid character found, return 0
        }
        input++;
    }
    return 1; // All characters are valid, return 1
}

// Check if a string consists only of digit characters
int is_all_digits(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0; // Found a non-digit character, return 0
        }
        str++;
    }
    return 1; // All characters are digits, return 1
}

// Main program function
int main(int argc, char *argv[]) {
    // Validate all command-line arguments
    for (int i = 1; i < argc; i++) {
        if (!validate_input(argv[i])) {
            exit(EXIT_FAILURE);
        }
    }

    int sockfd;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int use_tls = 0;
    char *server_name = NULL, *username = NULL, *password = NULL, *folder = "INBOX";
    char *command = NULL, *msgNum = NULL;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i+1 < argc) {
            username = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            password = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0 && i+1 < argc) {
            folder = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0 && i+1 < argc) {
            if (!is_all_digits(argv[i+1])) {
                exit(EXIT_FAILURE);
            }
            msgNum = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0) {
            use_tls = 1;
        } else {
            // Determine if parameter is a command or server name
            if (command == NULL) {
                command = argv[i];
            } else if (server_name == NULL) {
                server_name = argv[i];
            } else {
                exit(EXIT_FAILURE); // Unexpected parameter
            }
        }
    }

    // Validate essential parameters
    if (!server_name || !username || !password || !command) {
        exit(EXIT_FAILURE);
    }

    // Setup secure connection if TLS is required
    if (use_tls) {
        initialize_openssl();
        ctx = create_context();
        ssl = create_ssl_socket(server_name, ctx);
    } else {
        sockfd = create_socket(server_name);
    }

    // Read and write buffers for communication with server
    char read_buffer[2048];
    char write_buffer[2048];

    // Authenticate with server
    if (use_tls) {
        ssl_receive_msg(ssl, read_buffer, 2048);
    } else {
        receive_msg(sockfd, read_buffer, 2048);
    }

    snprintf(write_buffer, 2048, "A01 LOGIN %s %s\r\n", username, password);
    if (use_tls) {
        ssl_send_msg(ssl, write_buffer);
        ssl_receive_msg(ssl, read_buffer, 2048);
    } else {
        send_msg(sockfd, write_buffer);
        receive_msg(sockfd, read_buffer, 2048);
    }
    if (strstr(read_buffer, "A01 OK") == NULL) {
        printf("Login failure\n");
        exit(3);
    }

    // Send command to select mail folder
    if (strchr(folder, ' ') != NULL) {
        snprintf(write_buffer, sizeof(write_buffer), "A02 SELECT \"%s\"\r\n", folder);
    } else {
        snprintf(write_buffer, sizeof(write_buffer), "A02 SELECT %s\r\n", folder);
    }
    if (use_tls) {
        ssl_send_msg(ssl, write_buffer);
        ssl_receive_msg(ssl, read_buffer, 2048);
    } else {
        send_msg(sockfd, write_buffer);
        receive_msg(sockfd, read_buffer, 2048);
    }
    if (strstr(read_buffer, "A02 OK") == NULL) {
        printf("Folder not found\n");
        exit(3);
    }

    // Handle different commands based on user input
    if (strcmp(command, "retrieve") == 0) {
        handle_retrieve(use_tls ? ssl : NULL, sockfd, msgNum);
        exit(0);
    } else if (strcmp(command, "parse") == 0) {
        handle_parse(use_tls ? ssl : NULL, sockfd, msgNum);
    } else if (strcmp(command, "mime") == 0) {
        handle_mime(use_tls ? ssl : NULL, sockfd, msgNum);
    } else if (strcmp(command, "list") == 0) {
        handle_list(use_tls ? ssl : NULL, sockfd);
    }

    // Clean up TLS or socket connection
    if (use_tls) {
        cleanup_openssl(ssl, ctx);
    } else {
        close(sockfd);
    }
}
