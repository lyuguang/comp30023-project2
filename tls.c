#include "task.h"

// Initialize OpenSSL library
void initialize_openssl() {
    SSL_load_error_strings(); // Load SSL error strings
    OpenSSL_add_all_algorithms(); // Load all OpenSSL algorithms
    SSL_library_init(); // Initialize SSL library
}

// Create and initialize SSL context
SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_client_method(); // Use TLS client method
    ctx = SSL_CTX_new(method); // Create new SSL context
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr); // Print SSL errors
        exit(EXIT_FAILURE);
    }
    
    // Load the trusted CA certificates
    if (SSL_CTX_load_verify_locations(ctx, "/usr/local/share/ca-certificates/ca.crt", NULL) != 1) {
        fprintf(stderr, "Error loading trust store\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Create and connect SSL socket
SSL *create_ssl_socket(char *serverName, SSL_CTX *ctx) {
    char *port = "993"; // IMAPS port
    int sockfd, s;
    struct addrinfo hints, *servinfo, *p;
    SSL *ssl;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Use stream socket

    // Get address information for the server
    s = getaddrinfo(serverName, port, &hints, &servinfo);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(2); // Exit with error code 2
    }

    // Iterate through address information list
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        // Connect to the server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) {
            ssl = SSL_new(ctx); // Create new SSL structure
            SSL_set_fd(ssl, sockfd); // Set SSL file descriptor
            if (SSL_connect(ssl) == 1) { // Initiate SSL handshake
                freeaddrinfo(servinfo); // Free address information
                return ssl; // Return SSL structure
            }
            SSL_free(ssl); // Free SSL structure if connection fails
        }
        close(sockfd); // Close socket if connection fails
    }
    
    freeaddrinfo(servinfo); // Free address information
    fprintf(stderr, "Unable to connect\n");
    exit(2); // Exit with error code 2
}

// Send message through SSL socket
int ssl_send_msg(SSL *ssl, char *content) {
    if (SSL_write(ssl, content, strlen(content)) < 0) { // Write data to SSL socket
        perror("Error writing to socket");
        return -1;
    }
    return 0;
}

// Receive message from SSL socket
int ssl_receive_msg(SSL *ssl, char *buffer, size_t size) {
    int n = SSL_read(ssl, buffer, size - 1); // Read data from SSL socket
    if (n < 0) {
        perror("Error reading from socket");
        exit(3);
    }
    buffer[n] = '\0'; // Null-terminate received data
    return 0;
}

// Cleanup SSL and SSL context
void cleanup_openssl(SSL *ssl, SSL_CTX *ctx) {
    SSL_shutdown(ssl); // Shutdown SSL connection
    SSL_free(ssl); // Free SSL structure
    SSL_CTX_free(ctx); // Free SSL context
}
