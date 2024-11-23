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
char* string = "/add?value=10...."
int i = 0
while (string[i] != '?') {
    i++;
} 
int *point = &i;                 //this will give me the memory address of the pointer of the value at i
int *point_next = point + 1;     //this will give you the pointer of the next memory address after i
char value = ;             //this will give you the value of what point is pointing at: value at i
int address = &point;