The Chat structure contains an integer id, pointers (user, message, and timestamp), int num, pointer to a Reaction array. 
2 int variables, id and num_reactions (4 + 4 = 8 bytes) 
3 pointers (8 bytes each = 24 bytes) 
Reaction pointer (8 bytes)
Structurally = 40 bytes 
In each chat, if we assume:
User: 16 bytes
Message: 16 bytes
Timestamp: 18 bytes 
Total string memory :  50 bytes
40+50 = 90 bytes
Calculating total memory usage for chats for 10 chats would be 10 * 90 = 900 bytes. 100 chats : 9,000 bytes. 1000 chats: 90,000 bytes. One approach I can think o to lower the memory usage size would be to reduce the structure size. This would work because the structure size is the same each time a chat is created, so reducing the size we need for each chat structure will lower the storage usage overall as chats increases
If the constraints were removed, users could send longer messages. However, unrestricted message lengths can increase the memory requirement needed to run the server. This could also lead to performance issues when processing and storing the longer messages. If we made it so we could handle longer messages using dynamic memory allocation, it would increase the complexity of the function.
﻿
