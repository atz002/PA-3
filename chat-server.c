#include "http-server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include "chats.h"


char const HTTP_404_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
char const HTTP_200_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
#define TIMESTAMP_LENGTH 20 // "YYYY-MM-DD HH:MM:SS" + 1 (null char)

uint32_t max_reactions = 100;
uint32_t max_chats = 100000;

void url_decode(char *dest, const char *src) {
    printf("src: %s\n", src);
    while (*src) {
        if (*src == '%' && isxdigit(*(src + 1)) && isxdigit(*(src + 2))) {
            char hex[3] = { *(src + 1), *(src + 2), '\0' };
            *dest++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

void handle_404(int client_sock, char *path)
{
    /* Study notes:
    You should know
    1. what a file descriptor is-- what si the FD in this function (for example?)
    2. write system call (you should memorize the function paraemters)
    */
    printf("SERVER LOG: Got request for unrecognized path \"%s\"\n", path);

    char response_buff[BUFFER_SIZE];
    snprintf(response_buff, BUFFER_SIZE, "Error 404:\r\nUnrecognized path \"%s\"\r\n", path);
    // snprintf includes a null-terminator
    write(client_sock, HTTP_404_NOT_FOUND, strlen(HTTP_404_NOT_FOUND));

    // TODO: send response back to client?
    write(client_sock, response_buff, strlen(response_buff));
}
void handle_chat_response(int client_sock, Chat *chats, uint32_t num_chats) {
    // Write the HTTP header directly to the client
    const char *header = HTTP_200_OK;
    write(client_sock, header, strlen(header));

    // Iterate over each chat message and write it to the client
    for (uint32_t i = 0; i < num_chats; i++) {
        Chat *chat = &chats[i];

        // Format the main chat message
        char chat_message[256];
        int message_len = snprintf(chat_message, sizeof(chat_message),
                                   "[#%u %s] %s: %s\n",
                                   chat->id, chat->timestamp, chat->user, chat->message);

        // Write the formatted chat message directly to the client
        write(client_sock, chat_message, message_len);

        // Iterate over each reaction and write it to the client
        for (int j = 0; j < chat->num_reactions; j++) {
            Reaction *reaction = &chat->reactions[j];

            // Format the reaction message
            char reaction_message[128];
            int reaction_len = snprintf(reaction_message, sizeof(reaction_message),
                                        "    %s (%s)\n",
                                        reaction->user, reaction->message);

            // Write the formatted reaction message directly to the client
            write(client_sock, reaction_message, reaction_len);
        }
    }
}

char const HTTP_500_GENERAL_ERROR[] = "HTTP/1.1 500 Invalid Input or Size Overload of Reactions or Posts\r\nContent-Type: text/plain\r\n\r\n";

void handle_500(int client_sock, char *path)
{
    /* Study notes:
    You should know
    1. what a file descriptor is-- what si the FD in this function (for example?)
    2. write system call (you should memorize the function paraemters)
    */
    printf("SERVER LOG: path contains input errors\"%s\"\n", path);

    char response_buff[BUFFER_SIZE];
    snprintf(response_buff, BUFFER_SIZE, "Error 500:\r\nInvalid Input or Size Overload of Reactions or Posts \"%s\"\r\n", path);
    // snprintf includes a null-terminator
    write(client_sock, HTTP_500_GENERAL_ERROR, strlen(HTTP_500_GENERAL_ERROR));

    // TODO: send response back to client?
    write(client_sock, response_buff, strlen(response_buff));
}

// Function to create a new timestamp and add it to a Chat struct
void add_timestamp_to_chat(Chat *chat)
{
    time_t raw_time;
    struct tm *time_info;
    char temp_timestamp[TIMESTAMP_LENGTH];

    // Get the current time and check for errors
    if (time(&raw_time) == -1) {
        perror("time() failed");
        exit(1);
    }

    // Convert to local time and check for errors
    time_info = localtime(&raw_time);
    if (time_info == NULL) {
        perror("localtime() failed");
        exit(1);
    }

    // Format timestamp and check for success
    if (strftime(temp_timestamp, sizeof(temp_timestamp), "%Y-%m-%d %H:%M:%S", time_info) == 0) {
        fprintf(stderr, "strftime() failed to format time\n");
        exit(1);
    }

    // Add to heap and place in Chat struct
    // Study note: always (1) malloc BEFORE (2) assigning , using strcpy, etc.

    // Allocate memory for timestamp in Chat struct and check for success
    chat->timestamp = malloc(strlen(temp_timestamp) + 1);
    if (chat->timestamp == NULL) {
        perror("malloc() failed");
        exit(1);
    }

    // Copy formatted timestamp into the allocated space
    strcpy(chat->timestamp, temp_timestamp);
}

void add_reaction(Chat *chat, const char *reaction_user, const char *reaction_message)
{

    // Reallocate memory for one more reaction
    chat->reactions = realloc(chat->reactions, (chat->num_reactions + 1) * sizeof(Reaction));

    // Initialize the new reaction
    // Study note: don't read the & until u finish chat->reactionblah blah. & modifies the WHOLE line, not just &chat

    // Study note: imagine what happens if chat->num_reactions is 0 (and we want to add one reaction) vs when chat->num_reactions is 5 and we wnat to add 6
    Reaction *new_reaction = &chat->reactions[chat->num_reactions];
    new_reaction->user = malloc(strlen(reaction_user) + 1);

    strcpy(new_reaction->user, reaction_user);

    new_reaction->message = malloc(strlen(reaction_message) + 1);

    strcpy(new_reaction->message, reaction_message);

    // Increment the number of reactions
    chat->num_reactions++;
}

// Create and add a new Chat struct to an array on the heap
// You need to pass the ADDRESS of num_chats such that the function odesn't create a COPY (i.e. it's basically a global variable)
void add_new_chat(Chat **chats, uint32_t *num_chats, const char *user, const char *message)
{
    // Reallocate memory for the chat array to add a new element
    // Study note: MAYBE you need to know perror and exit.

    *chats = realloc(*chats, (*num_chats + 1) * sizeof(Chat));
    if (*chats == NULL) {
        perror("Failed to allocate memory for chats");
        exit(1);
    }

    // Study Note:  (*pointer).attribute1  is the same thing as pointer->attribute1... this way we don't need to do parenthesis + *variable
    // (1) because we realloced the *chat to be ONE bigger (+1), the line after this comment is the MEMORY ADDRESS of an EMPTY chat

    // Initialize new Chat struct
    Chat *new_chat_msg = &(*chats)[*num_chats];
    // Possible change: Chat *new_chat_msg = &(chats[**num_chats]);
    // new_chat_msg = 0x123

    new_chat_msg->id = *num_chats + 1;

    // Allocate and copy the user
    new_chat_msg->user = malloc(strlen(user) + 1);
    strcpy(new_chat_msg->user, user);

    // Allocate and copy the message
    new_chat_msg->message = malloc(strlen(message) + 1);
    strcpy(new_chat_msg->message, message);

    // Add a timestamp to the chat
    add_timestamp_to_chat(new_chat_msg);

    // Initialize the reactions array (allocate 0 elements initially)
    new_chat_msg->num_reactions = 0;
    new_chat_msg->reactions = malloc(0);

    // Increment the number of chats
    (*num_chats)++;
}

// Function that frees all chat messages and reactions from heap
void free_chats(Chat *chats, uint32_t num_chats)
{
    for (uint32_t i = 0; i < num_chats; i++)
    {
        free(chats[i].user);
        free(chats[i].message);
        free(chats[i].timestamp);
        // Free each reaction within the chat
        for (uint32_t j = 0; j < chats[i].num_reactions; j++)
        {
            free(chats[i].reactions[j].user);
            free(chats[i].reactions[j].message);
        }
        free(chats[i].reactions); // Free the reactions array
    }
    free(chats); // Free the array of Chat structs
}

void handle_response(char *request, int client_sock, Chat **chats, uint32_t *num_chats) {
    char path[256];
    printf("SERVER LOG: Received request: \"%s\"\n", request);

    // printf("SERVER LOG: Got request for path \"%s\"\n", request);
    // example of request
    // GET /post?user=joe&message=hi aaron HTTP/1.1
    // User-Agent: curl/7.29.0
    // Host: ieng6-201:8080
    // Accept: */*

    // Parse the path out of the request line
    char *start_of_path = request + 4;
    char *end_of_path = strstr(start_of_path, " HTTP/1.1");
    if (end_of_path == NULL) {
        printf("SERVER LOG: Invalid request line\n");
        handle_404(client_sock, path);
        return;
    }

    int path_length = end_of_path - start_of_path;
    if (path_length >= sizeof(path) - 1) {
        printf("SERVER LOG: Path too long\n");
        handle_404(client_sock, path);
        return;
    }

    strncpy(path, start_of_path, path_length);
    path[path_length] = '\0';
    printf("SERVER LOG: Parsed path \"%s\"\n", path);

    char *query_start = strchr(path, '?');
    int endpoint_length = query_start ? query_start - path : strlen(path);
    char endpoint[256];
    strncpy(endpoint, path, endpoint_length);
    endpoint[endpoint_length] = '\0';
    printf("SERVER LOG: Parsed endpoint \"%s\"\n", endpoint);

    const char *poststr = "/post";
    const char *reactstr = "/react";

    if (strcmp(endpoint, poststr) == 0) {

        // uses malloc to increase the size of the amount of
        // chat messages using the Chat struct each time a new
        // chat message is received and keeps track of the amount of chat messages
        // Safely parse user and message parameters
        char user[256] = {0}, message[256] = {0};

        if (query_start) {
            char *user_start = strstr(query_start, "user=");
            char *message_start = strstr(query_start, "&message=");

            if (user_start && message_start) {
                user_start += 5;
                int user_len = message_start - user_start;
                if (user_len > 0 && user_len < sizeof(user)) {
                    strncpy(user, user_start, user_len);
                    user[user_len] = '\0';
                }

                message_start += 9;
                strncpy(message, message_start, sizeof(message) - 1);
                message[sizeof(message) - 1] = '\0';
            } else {
                printf("SERVER LOG: Missing parameters in POST request\n");
                handle_404(client_sock, path);
                return;
            }
        }

        url_decode(user, user);
        url_decode(message, message);
        printf("SERVER LOG: Decoded user \"%s\" and message \"%s\"\n", user, message);

        add_new_chat(chats, num_chats, user, message);

        printf("SERVER LOG: Added new chat. Total chats: %u\n", *num_chats);
        handle_chat_response(client_sock, *chats, *num_chats);
    }
    else if (strcmp(endpoint, reactstr) == 0) {
        // Safely parse user, message, and ID parameters
        char user[256] = {0}, message[256] = {0};
        int id = -1;

        if (query_start) {
            char *user_start = strstr(query_start, "user=");
            char *message_start = strstr(query_start, "&message=");
            char *id_start = strstr(query_start, "&id=");

            if (user_start && message_start && id_start) {
                user_start += 5;
                int user_len = message_start - user_start;
                if (user_len > 0 && user_len < sizeof(user)) {
                    strncpy(user, user_start, user_len);
                    user[user_len] = '\0';
                }

                message_start += 9;
                int message_len = id_start - message_start;
                if (message_len > 0 && message_len < sizeof(message)) {
                    strncpy(message, message_start, message_len);
                    message[message_len] = '\0';
                }

                id_start += 4;
                id = atoi(id_start);
            } else {
                printf("SERVER LOG: Missing parameters in REACT request\n");
                handle_404(client_sock, path);
                return;
            }
        }

        url_decode(user, user);
        url_decode(message, message);
        printf("SERVER LOG: Decoded user \"%s\", message \"%s\", and ID %d\n", user, message, id);

        if (id <= 0 || id > *num_chats) {
            printf("SERVER LOG: Invalid ID %d in REACT request\n", id);
            handle_404(client_sock, path);
            return;
        }

        add_reaction(chats[id - 1], user, message);
        printf("SERVER LOG: Added reaction to chat ID %d. Total reactions for chat %d: %u\n", id, id, (*chats)[id - 1].num_reactions);
        handle_chat_response(client_sock, *chats, *num_chats);
    }
    else {
        printf("SERVER LOG: Unrecognized path \"%s\"\n", path);
        handle_404(client_sock, path);
    }
}


int main(int argc, char *argv[])
{
    int port = (argc >= 2) ? atoi(argv[1]) : 0;
    Chat *chats = NULL;
    uint32_t num_chats = 0;

    start_server(handle_response, port, &chats, &num_chats);

    free_chats(chats, num_chats);

    return 0;
}
