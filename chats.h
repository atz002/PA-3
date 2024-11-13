// chat.h
#ifndef CHAT_H
#define CHAT_H

#include <stdint.h>

typedef struct Reaction {
    char *user;
    char *message;
} Reaction;

typedef struct Chat {
    uint32_t id;
    char *user;
    char *message;
    char *timestamp;
    uint32_t num_reactions;
    Reaction *reactions;
} Chat;

#endif // CHAT_H
