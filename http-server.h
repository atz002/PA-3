#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "chats.h"

void start_server(void (*handler)(char *, int, Chat **, uint32_t *), int port, Chat **chats, uint32_t *num_chats);

#define BUFFER_SIZE 2048

#endif
