Chat gtp:
Q:
when do we use malloc or realloc for this part? im not quite sure i understand the difference.
When we use realloc, what happens tot he memory of the previous pointer? Meaning, if chat->reactions previously had sizeof(Reaction) * 3, and the adddresses were filled with information, when we REALLOC, what happens to that previous infor. Does it get erased or stay the same. 
A:
When you use realloc, the contents of the original memory block (pointed to by chat->reactions) are preserved up to the minimum of the old and new sizes. This means that if you had previously allocated memory for, say, 3 Reaction structs, and now you're increasing it to 4, the first 3 Reaction entries remain intact after realloc.
 // Expand array to hold 4 Reaction structs
    reactions = realloc(reactions, 4 * sizeof(Reaction));
    reactions[3].id = 4; // New reaction

Q:
HOw do I grab a timestamp of the current time? explain how we are using this library to impliment it into our code

A:
void add_timestamp_to_chat(Chat *chat)
{

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

Q: What am i doing wrong when freeing my chats? Is the order in which I am doing it? 

A:
void free_chats(Chat *chats, uint32_t num_chats)
{
    for (uint32_t i = 0; i < num_chats; i++)
    {
        free(chats[i].user);
        free(chats[i].message);
        free(chats[i].timestamp);
        for (uint32_t j = 0; j < chats[i].num_reactions; j++)
        {
            free(chats[i].reactions[j].user);
            free(chats[i].reactions[j].message);
        }
        free(chats[i].reactions);
    }
    free(chats);
}