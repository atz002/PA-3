#include "http-server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

char const HTTP_404_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";

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

#define TIMESTAMP_LENGTH 17 // "YYYY-MM-DD HH:MM" + 1 (null char)

typedef struct Reaction
{
    char *user;
    char *message;
} Reaction; // Add this semicolon

typedef struct Chat
{
    uint32_t id;
    char *user;
    char *message;
    char *timestamp;
    uint32_t num_reactions;
    Reaction *reactions;
} Chat; // Add this semicolon

Chat *chats = NULL;
uint32_t num_chats = 0;

uint32_t max_reactions = 100;
uint32_t max_chats = 100000;

// Function to create a new timestamp and add it to a Chat struct
void add_timestamp_to_chat(Chat *chat)
{
    /* Study notes:
    You  should know how to extract timestamp in various ways just in case midterm tests it. Practice writing this out without looking

    1. time_t is a data type used in C to represent a point in time. It typically stores the number of seconds that have passed since the start of the Unix epoch (January 1, 1970, at 00:00:00 Coordinated Universal Time (UTC)).

    */
    // Get the current time
    time_t raw_time;
    struct tm *time_info;
    char temp_timestamp[TIMESTAMP_LENGTH];

    // Get the current time, format as "YYYY-MM-DD HH:MM"
    time(&raw_time);                  // time(BUFFER) function grabs the number of seconds since 1970, and puts it into the BUFFER, which in our case is the & of raw_time
    time_info = localtime(&raw_time); // localtime(seconds since 1970) will basically reformat it into some "structure".
    strftime(temp_timestamp, sizeof(temp_timestamp), "%Y-%m-%d %H:%M", time_info);

    // Add to heap and place in Chat struct
    // Study note: always (1) malloc BEFORE (2) assigning , using strcpy, etc.
    chat->timestamp = malloc(strlen(temp_timestamp) + 1);

    // Copy formatted timestamp into the heap
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
Chat *add_new_chat(Chat **chats, uint32_t *num_chats, const char *user, const char *message)
{
    // Reallocate memory for the chat array to add a new element
    // Study note: really understand why use *chats instead of just "chats"
    *chats = realloc(*chats, (*num_chats + 1) * sizeof(Chat));
    if (*chats == NULL)
    {
        // Study note: MAYBE you need to know perror and exit.
        perror("Failed to allocate memory for chats");
        exit(1);
    }

    // Initialize new Chat struct
    // Study Note:  (*pointer).attribute1  is the same thing as pointer->attribute1... this way we don't need to do parenthesis + *variable
    // (1) because we realloced the *chat to be ONE bigger (+1), the line after this comment is the MEMORY ADDRESS of an EMPTY chat
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
    new_chat_msg->reactions = malloc(0); // Allocating empty array

    // Increment the number of chats
    (*num_chats)++;

    return new_chat_msg;
}

// Function that frees all chat messages and reactions from heap
void free_chats(Chat *chats, uint32_t num_chats)
{
    for (uint32_t i = 0; i < num_chats; i++)
    {
        free(chats[i].user);
        free(chats[i].message);
        free(chats[i].timestamp);
        // Study note: notice how we free the "user" and "message" of each reaction BEFORE freeing the "reaction" itself
        for (uint32_t j = 0; j < chats[i].num_reactions; j++)
        {
            free(chats[i].reactions[j].user);
            free(chats[i].reactions[j].message);
        }
        free(chats[i].reactions); // <--
    }
    free(chats);
}

void handle_response(char *request, int client_sock)
{

    // printf("\nSERVER LOG: Got request: \"%s\"\n", request);

    // Parse the path out of the request line (limit buffer size; sscanf null-terminates)
    // if (sscanf(request, "GET %255s", path) != 1)
    // {
    //     printf("Invalid request line\n");
    //     return;
    // }

    // printf("SERVER LOG: Got request for path \"%s\"\n", request);
    // example of request
    // GET /post?user=joe&message=hi aaron HTTP/1.1
    // User-Agent: curl/7.29.0
    // Host: ieng6-201:8080
    // Accept: */*

    // Chat *chats = NULL;
    // uint32_t num_chats = 0;

    char path[256];

    // Assume the request starts with "GET ", so the path begins at offset 4
    char *start_of_path = request + 4;

    // Find the end of the path by searching for " HTTP/1.1"
    char *end_of_path = strstr(start_of_path, " HTTP/1.1"); // Starts at start_of_path and stops at " HTTP..."
                                                            // strstr gives me a pointer of where we stop looking
    if (end_of_path == NULL)
    {
        printf("Invalid request line\n");
        return;
    }

    // Calculate path length and check if it's within the 255 character limit
    int path_length = end_of_path - start_of_path;
    if (path_length >= 255)
    { // Limit to 255 characters to prevent overflow
        printf("Path too long\n");
        handle_404(client_sock, path);
        return;
    }

    // Copy the path into `path` buffer and null-terminate it
    strncpy(path, start_of_path, path_length);
    path[path_length] = '\0'; // Null-terminate the path

    printf("SERVER LOG: Got request for path \"%s\"\n", path);
    int endpoint_length;

    // Find the position of '?' to separate the endpoint
    char *query_start = strstr(path, "?");
    if (query_start == NULL)
    {
        // No query parameters, endpoint is the entire path
        endpoint_length = strlen(path);
    }
    else
    {
        // Query parameters found, endpoint is up to the '?'
        endpoint_length = query_start - path;
    }

    // Copy the endpoint into `endpoint` buffer and null-terminate it
    char endpoint[256];
    strncpy(endpoint, path, endpoint_length);
    endpoint[endpoint_length] = '\0'; // Null-terminate the endpoint

    // handle_404(client_sock, path);
    const char *chatstr = "/chats";
    const char *poststr = "/post";
    const char *reactstr = "/react";
    const char *resetstr = "/reset";
    if (strcmp(endpoint, chatstr) == 0)
    {
        printf("/chats\n");
        for (int i = 0; i < num_chats; i++)
        {
            // First print out time stamp
            Chat *chat = &chats[i]; // Study note: this line here is a SINGULAR chat (a pointer, so we can use the -> syntax) I want the ADDRESS of chats[i]

            // Print the chat message with ID, timestamp, user, and message content
            // 16s means WIDTH -- RIGHT ALIGNED
            printf("[#%u %s] %16s: %s\n", chat->id, chat->timestamp, chat->user, chat->message);

            // IF there are reactions -- print each reaction
            for (int j = 0; j < chat->num_reactions; j++)
            {
                Reaction *reaction = &chat->reactions[j];                          // Why use &? & modifes the WHOLE THING not just chat... I want the address of hte J-TH reaction
                printf(" %28s (%s)  %s\n", "", reaction->user, reaction->message); // indented line for reaction
            }
        }
    }
    else if (strcmp(endpoint, poststr) == 0)
    {
        printf("/post\n");
        // parses a string such as /post?user=<username>&message=<message> e.g. /post?user=joe&message=hi aaron
        // out of the path string and stores the username and message strings inside of variables here
        // checks if username is longer than 15 bytes and if not returns 404, stores it in variable char[15] username;
        // uses malloc to increase the size of the amount of chat messages using the Chat struct each time a new chat message is received and keeps track of the amount of chat messages
        // if this post request increases to more than 100000 (one hundred thousand) chats, the server should respond with an error (HTTP code 404 or 500)
        // If a parameter (username or message) is missing, respond with some kind of error (HTTP code 400 or 500)
        char user[256];
        char *user_start = query_start + 6;       // should start after =, point to j
        char *user_end = strstr(user_start, "&"); // should point to &, end before it reaches
        int user_len = user_end - user_start;
        if (user_len < 1)
        {
            printf("Username length was less than 1 character long \n");
            handle_404(client_sock, path);
            return;
        }
        strncpy(user, user_start, user_len);
        user[user_len] = '\0';

        char message[256];
        char *message_start = user_end + 9; // should point to right after =
        int message_len = 0;
        char *message_count = message_start;
        while (*message_count != '\0')
        {
            message_len++;
            message_count++;
        }
        if (message_len < 1)
        {
            printf("Message length was less than 1 character long \n");
            handle_404(client_sock, path);
            return;
        }
        strncpy(message, message_start, message_len);
        message[message_len] = '\0';

        printf("Name:\n%s \nMessage:\n%s\n", user, message);

        // IF MAX CHATS REACHED, RETURN
        if (num_chats + 1 > max_chats)
        {
            printf("maxchat: %s num_chat: %s ", max_chats, num_chats);
            handle_500(client_sock, request);
            return;
        }

        // calling add new chat with
        // (1) address of the array of chats.
        // (2) the NUMBER of chats
        // (3) pass in the user who is trying to add a chat
        // (4) pass the message itself
        add_new_chat(&chats, &num_chats, user, message); // should malloc/realloc a new chat into chats array.
        printf("New chat message added, total chat messages and usernames + messages in each chat msg: \n");
        printf("DEBUG: %d\n", num_chats);
        for (int i = 0; i < num_chats; i++)
        {
            printf("Chat idx: %d, user: %s, msg: %s, id: %d\n", i, chats[i].user, chats[i].message, chats[i].id);
            printf("Reaction Messages: \n");
            if (chats[i].num_reactions == 0)
            {
                printf("No reactions\n");
                continue;
            }
            for (int j = 0; j < chats[i].num_reactions; j++)
            {
                printf("Reaction %d: %s: %s\n", j, chats[i].reactions[j].user, chats[i].reactions[j].message);
            }
        }
    }
    else if (strcmp(endpoint, reactstr) == 0)
   {
    printf("/react\n");

    // Extract user parameter
    char user[256];
    char *user_start = query_start + 6;       // After "user="
    char *user_end = strstr(user_start, "&");
    if (user_end == NULL) {
        printf("Error: 'user' parameter missing or incorrectly formatted in /react path\n");
        handle_404(client_sock, path);
        return;
    }
    int user_len = user_end - user_start;
    if (user_len < 1 || user_len >= 256) {
        printf("Error: 'user' parameter length invalid (0 or exceeds 255 characters)\n");
        handle_404(client_sock, path);
        return;
    }
    strncpy(user, user_start, user_len);
    user[user_len] = '\0';

    // Extract message parameter
    char message[256];
    char *message_start = user_end + 9; // After "message="
    char *message_end = strstr(message_start, "&");
    int message_len;
    if (message_end == NULL) {
        message_len = strlen(message_start); // Message is at the end of URL
    } else {
        message_len = message_end - message_start;
    }
    if (message_len < 1 || message_len >= 256) {
        printf("Error: 'message' parameter length invalid (0 or exceeds 255 characters)\n");
        handle_404(client_sock, path);
        return;
    }
    strncpy(message, message_start, message_len);
    message[message_len] = '\0';

    // Extract id parameter
    char *id_start = message_end ? message_end + 4 : NULL; // After "id="
    if (id_start == NULL || strlen(id_start) == 0) {
        printf("Error: 'id' parameter missing or incorrectly formatted in /react path\n");
        handle_404(client_sock, path);
        return;
    }
    int id = atoi(id_start);
    if (id <= 0 || id > num_chats) {
        printf("Error: 'id' parameter out of valid range. id: %d, num_chats: %u\n", id, num_chats);
        handle_500(client_sock, path);
        return;
    }

    // Check reaction limit for the chat
    if (chats[id - 1].num_reactions >= max_reactions) {
        printf("Error: Exceeded maximum reactions for chat ID %d\n", id);
        handle_500(client_sock, path);
        return;
    }

    // Add reaction
    add_reaction(&chats[id - 1], user, message);
    printf("Reaction added: User: %s, Message: %s, Chat ID: %d\n", user, message, id);
}
    else if (strcmp(endpoint, resetstr) == 0)
    {
        printf("/reset\n");
        free_chats(chats, num_chats);
        chats = NULL;

        num_chats = 0;
    }
    else
    {
        printf("Invalid path\n");
    }
}

int main(int argc, char *argv[])
{
    int port = 0;
    if (argc >= 2)
    { // if called with a port number, use that
        port = atoi(argv[1]);
    }
    else if (argc == 1)
    {
        port = 0; // should automatically assigned if port is 0
    }

    start_server(&handle_response, port);

    free_chats(chats, num_chats);

    return 0;
}
