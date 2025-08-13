#include "task.h"

// Case-insensitive string search
char *stristr(const char *haystack, const char *needle) {
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
void process_header_field(char *read_buffer, const char *field_name) {
    char *field_start = stristr(read_buffer, field_name);

    // Print "<No subject>" if the field is "Subject" and not found
    if (strcmp(field_name, "Subject:") == 0 && field_start == NULL) {
        printf("Subject: <No subject>\n");
        return;
    }

    // Print "To:" if the field is "To" and not found
    if (strcmp(field_name, "To:") == 0 && field_start == NULL) {
        printf("To:\n");
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
        printf("%s", field_name);
        printf("%.*s\n", (int)(field_end - field_start - strlen(field_name)), clean_buffer + strlen(field_name));

        // Move to the next occurrence of the field if there is one
        field_start = stristr(field_end, field_name);
    }
}

void handle_parse(SSL *ssl, int sockfd, char *msgNum) {
    char write_buffer[2048];
    char read_buffer[2048];

    // Fetching "From" header fields
    snprintf(write_buffer, 2048, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (FROM)]\r\n", msgNum);
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        send_msg(sockfd, write_buffer);
    }
    // send_msg(sockfd, write_buffer);
    receive_msg(sockfd, read_buffer, 2048);
    process_header_field(read_buffer, "From:");

    // Fetching "To" header fields
    snprintf(write_buffer, 2048, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (TO)]\r\n", msgNum);
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        send_msg(sockfd, write_buffer);
    }
    // send_msg(sockfd, write_buffer);
    receive_msg(sockfd, read_buffer, 2048);
    process_header_field(read_buffer, "To:");

    // Fetching "Date" header fields
    snprintf(write_buffer, 2048, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (DATE)]\r\n", msgNum);
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        send_msg(sockfd, write_buffer);
    }
    // send_msg(sockfd, write_buffer);
    receive_msg(sockfd, read_buffer, 2048);
    process_header_field(read_buffer, "Date:");

    // Fetching "Subject" header fields
    snprintf(write_buffer, 2048, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", msgNum);
    if (ssl) {
        ssl_send_msg(ssl, write_buffer);
    } else {
        send_msg(sockfd, write_buffer);
    }
    // send_msg(sockfd, write_buffer);
    receive_msg(sockfd, read_buffer, 2048);
    process_header_field(read_buffer, "Subject:");
}
    
