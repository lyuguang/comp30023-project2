#include "task.h"

void extract_boundary(char boundary[], char line[]) {
    char *start = strchr(line, '"');
    if (start != NULL) {
        // If quotation marks are found, extract the content within the quotation marks
        start++;
        char *end = strchr(start, '"');
        if (end != NULL) {
            strncpy(boundary + 2, start, end - start);
            boundary[end - start + 2] = '\0';
            return;
        }
    }
    
    // If no quotation marks are found, the content of the boundary parameter is extracted directly
    strcpy(boundary + 2, line);
    boundary[strlen(line) + 2] = '\0';
}

void handle_mime(SSL *ssl, int sockfd, char *msgNum) {
    char write_buffer[2048];
    snprintf(write_buffer, 2048, "A03 FETCH %s BODY.PEEK[]\r\n", msgNum);
    ssize_t n;
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        n = send_msg(sockfd, write_buffer);
    }

    // ssize_t n = send_msg(sockfd, write_buffer);
    if (n < 0)
    {
        printf("write error\n");
        exit(3);
    }
    
    char one_line[10000] = {0};
    char print_line[10000];
    char header_line[10000] = {0};
    char *token = " boundary=";
    int count = 0;
    char boundary[10000] = {0};
    boundary[0] = '-';
    boundary[1] = '-';
    int start_body = 0;
    int end_body = 0;
    int is_target_part = 0;
    int is_plain_text_part = 0;
    int skip_first_empty_line = 1;

    while (1)
    {
        //read one line
        count = 0;
        while (1)
        {
            char c;
            ssize_t bytes_read = read(sockfd, &c, sizeof(char));
            if (bytes_read <= 0) {
                end_body = 1;
                break;
            }
            one_line[count] = c;
            count++;
            if (c == '\n')
            {
                one_line[count] = '\0';
                break;
            }
        }

        if (end_body)
        {
            break;
        }
        
        if (strncmp(one_line, token, strlen(token)) == 0)
        {
            extract_boundary(boundary, one_line + strlen(token));
        }

        // check if the line is the beginning or the end of the body
        if (strncmp(one_line, boundary, strlen(boundary)) == 0)
        {
            if (!start_body) {
                start_body = 1;
            } else {
                if (is_plain_text_part) {
                    // Find the location of the carriage return
                    char *newline = strchr(print_line, '\r');
                    if (newline != NULL) {
                        // Replace the carriage return character with the end symbol of the string
                        *newline = '\0';
                    }

                    // Print the contents of print_line after removing the carriage return.
                    printf("%s", print_line);
                    break;
                }
                is_target_part = 0;
                is_plain_text_part = 0;
            }
            continue; 
        }

        // Check if target MIME part
        if (start_body && !is_target_part) {
            // If the current line starts with a space or tab, append it to header_line
            if (one_line[0] == ' ' || one_line[0] == '\t') {
                strcat(header_line, one_line);
                continue;
            }

            // If header_line is non-empty, perform a match check
            if (header_line[0] != '\0') {
                if (strstr(header_line, "Content-Transfer-Encoding") != NULL || strstr(header_line, "MIME-Version") != NULL ||
                    (strstr(header_line, "Content-Type") != NULL && strstr(header_line, "charset=UTF-8") != NULL) ||
                    strstr(header_line, "Content-Description") != NULL || strstr(header_line, "Content-ID") != NULL) {
                    is_target_part = 1;
                    is_plain_text_part = 1;
                    skip_first_empty_line = 1;
                }
                header_line[0] = '\0';  // Clear header_line
            }

            // Copy the current line into header_line for the next match
            strcpy(header_line, one_line);
            continue;
        }
        // printf("%s\n", boundary);

        if (is_target_part && is_plain_text_part) {
            // Skip header fields
            if (skip_first_empty_line && strcmp(one_line, "\r\n") == 0) {
                skip_first_empty_line = 0;
                continue;
            } 

            //printf("%s", one_line);
            if (strlen(print_line) > 0) {
                printf("%s", print_line);
            }

            strcpy(print_line, one_line);
        }
    }
}

