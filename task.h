#ifndef TASK_H
#define TASK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

int create_socket(char *serverName);
int receive_msg(int sockfd, char *buffer, size_t size);
int send_msg(int sockfd, char *content);

void handle_retrieve(SSL *ssl, int sockfd, char *msgNum);

void handle_parse(SSL *ssl, int sockfd, char *msgNum);
char *stristr(const char *haystack, const char *needle);
void process_header_field(char *read_buffer, const char *field_name);

void handle_mime(SSL *ssl, int sockfd, char *msgNum);
void extract_boundary(char boundary[], char line[]);

void handle_list(SSL *ssl, int sockfd);

void initialize_openssl();
SSL_CTX *create_context();
SSL *create_ssl_socket(char *serverName, SSL_CTX *ctx);
void cleanup_openssl(SSL *ssl, SSL_CTX *ctx);
int ssl_receive_msg(SSL *ssl, char *buffer, size_t size);
int ssl_send_msg(SSL *ssl, char *content);

#endif