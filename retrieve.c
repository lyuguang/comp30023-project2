#include "task.h"

// Function to handle the retrieval of an email message
void handle_retrieve(SSL *ssl, int sockfd, char *msgNum) {
    char write_buffer[2048];
    // Construct the FETCH command to retrieve the email body
    snprintf(write_buffer, 2048, "A03 FETCH %s BODY.PEEK[]\r\n", msgNum);
    // Send the FETCH command
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        send_msg(sockfd, write_buffer);
    }

    char one_line[1000];
    char c;
    int num_char = 0;

    // Read response from server until newline character
    while (num_char < (int)(sizeof(one_line) - 1)) {
        if (ssl) {
            if (SSL_read(ssl, &c, sizeof(char)) <= 0) {
                break;
            }
        } else {
            if (read(sockfd, &c, sizeof(char)) <= 0) {
                break;
            }
        }

        one_line[num_char] = c;
        num_char++;

        if (c == '\n') {
            break;
        }
    }
    one_line[num_char] = '\0';

    // Check for error response from server
    if (strstr(one_line, "A03 BAD") != NULL) {
        printf("Message not found\n");
        exit(3);
    }

    int start = 0;
    int end = 0;

    // Find the total number of bytes in the email message
    for (size_t i = 0; i < strlen(one_line); i++) {
        if (one_line[i] == '{') {
            start = i + 1;
        }

        if (one_line[i] == '}') {
            end = i;
            break;
        }
    }

    one_line[end] = '\0';
    int total = atoi(one_line + start);

    // Read and print the email message
    while (total > 0) {
        if (ssl) {
            if (SSL_read(ssl, &c, sizeof(char)) <= 0) {
                break;
            }
        } else {
            if (read(sockfd, &c, sizeof(char)) <= 0) {
                break;
            }
        }

        printf("%c", c);
        total -= 1;
    }
}
