#include "task.h"

// Function to create a socket and connect it to the specified server
int create_socket(char *serverName) {
    char *port = "143"; // Default port for IMAP
    int sockfd, s;
    struct addrinfo hints, *servinfo, *p;

    // Clear the hints structure and set the relevant parameters
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Use stream socket

    // Get the server address information
    s = getaddrinfo(serverName, port, &hints, &servinfo);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Iterate through the server address information list and try to connect to the server
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        // Create a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        // Connect to the server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1)
        {
            break;
        }
        close(sockfd); // Close the socket if connection fails
    }
    
    // Check if connection to the host failed
    if (p == NULL)
    {
        printf("host failure\n");
        exit(3);
    }
    freeaddrinfo(servinfo); // Free the memory allocated for server address information
    return sockfd;
}

// Function to receive a message from the socket
int receive_msg(int sockfd, char *buffer, size_t size) {
    ssize_t n = read(sockfd, buffer, size - 1); // Read data from socket
    if (n < 0)
    {
        printf("Error reading from socket");
        exit(3);
    }
    buffer[n] = '\0'; // Null terminate the received data

    return 0;
}

// Function to send a message through the socket
int send_msg(int sockfd, char *content) {
    if (write(sockfd, content, strlen(content)) < 0) // Write data to socket
    {
        perror("Error writing to socket");
        return -1;
    }
    return 0;
}
