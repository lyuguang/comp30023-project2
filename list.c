#include "task.h"

// Case-insensitive string search
char *str_search(const char *haystack, const char *needle) {
    char *h, *n;
    if (!*needle) // Empty needle
        return (char *) haystack;
    while (*haystack) {
        h = (char *) haystack;
        n = (char *) needle;
        while (*h && *n && (tolower((unsigned char) *h) == tolower((unsigned char) *n))) {
            h++;
            n++;
        }
        if (!*n) // Found
            return (char *) haystack;
        haystack++;
    }
    return NULL; // Not found
}

// Function to process a header field
void process_header_field2(char *read_buffer, const char *field_name, const char *num) {
    char *field_start = str_search(read_buffer, field_name);

    // Print "<No subject>" if the field is "Subject" and not found
    if (strcmp(field_name, "Subject:") == 0 && field_start == NULL) {
        printf("%s: <No subject>\n", num);
        return;
    }

    // Process each occurrence of the field separately
    while (field_start != NULL) {
        char *field_end = strchr(field_start, '\n');
        char clean_buffer[2048]; // Store the processed string
        int i, j = 0;

        // Find the position of "\r\n\r\n" after the current field occurrence
        char *double_newline = strstr(field_start, "\r\n\r\n");
        if (double_newline != NULL) {
            // If "\r\n\r\n" is found, ignore the part after it
            *double_newline = '\0';
            field_end = double_newline; // Update field end to "\r\n\r\n"
        }

        // Iterate over field_start, and add non-newline and non-carriage return characters to clean_buffer
        for (i = 0; field_start[i] != '\0' && &field_start[i] != field_end; ++i) {
            if (field_start[i] != '\r' && field_start[i] != '\n') {
                clean_buffer[j++] = field_start[i];
            }
        }
        clean_buffer[j] = '\0'; // Add string terminator

        // Print the field name with original case and the processed string
        printf("%s:", num);
        printf("%.*s\n", (int)(field_end - field_start - strlen(field_name)), clean_buffer + strlen(field_name));

        // Move to the next occurrence of the field if there is one
        field_start = str_search(field_end, field_name);
    }
}

void handle_list(SSL *ssl, int sockfd) {
    char write_buffer[2048];
    snprintf(write_buffer, 2048, "A03 SEARCH ALL\r\n");
    ssize_t n;
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        n = send_msg(sockfd, write_buffer);
    }
    // ssize_t n = send_msg(sockfd, write_buffer);

    if (n < 0)
    {
        printf("write error");
        exit(3);
    }

    char read_buffer[2048];
    char new_read_buffer[2048];
    n = receive_msg(sockfd, read_buffer, 2048);

    char *searchPtr = strstr(read_buffer, "SEARCH");
    if (searchPtr != NULL) {
        searchPtr += strlen("SEARCH");
        while (*searchPtr != '\0' && *searchPtr != '\n') {
            if (*searchPtr >= '0' && *searchPtr <= '9') {
                char num[20]; // Assume a maximum of 20 characters
                int i = 0;
                while (*searchPtr != '\0' && *searchPtr != '\n' && (*searchPtr >= '0' && *searchPtr <= '9')) {
                    num[i++] = *searchPtr++;
                }
                num[i] = '\0'; // Add a terminator to the end of the string

                snprintf(write_buffer, 2048, "A04 FETCH %s BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", num);
                send_msg(sockfd, write_buffer);
                receive_msg(sockfd, new_read_buffer, 2048);
                process_header_field2(new_read_buffer, "Subject:", num);
            } else {
                searchPtr++;
            }
        }
    } else {
        exit(0);
    }
}