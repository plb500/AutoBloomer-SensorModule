#ifndef _MENU_INTERFACE_H_
#define _MENU_INTERFACE_H_

#include "pico/stdlib.h"

// Menu loop state
typedef enum {
    DO_NOTHING = 0,
    EXIT_MENU = 1,
    REPRINT_MENU_TEXT = 2
} menu_return_behavior;

// Buffer for reading user input
#define MAX_INPUT_LENGTH    (32)
typedef struct {
    char mBuffer[MAX_INPUT_LENGTH + 1];     // We add 1 for null-termination
    uint32_t mCurrentInputPos;
} UserInputBuffer;

typedef enum {
    INPUT_INCOMPLETE    = 0,
    INPUT_COMPLETE,
    INPUT_ABORTED
} UserInputState;

typedef struct {
    UserInputState mInputState;
    int32_t mIntValue;
} UserInputInt;

typedef enum {
    PROMPT_USER = 0,
    READING_INPUT,
    INPUT_COMPLETED

} UserInputInteractionState;

typedef struct {
    UserInputInteractionState mState;
    UserInputBuffer mArgumentBuffer;
} UserInputStateHolder;

// Read incoming user input and attempt to coerce it into an integer value
UserInputInt read_int_input(int input, UserInputBuffer *inputBuffer);

// Generic menu input handler function
typedef menu_return_behavior (*input_handler)(int, void*);

// Run the menu with the supplied text and input handling function
void do_menu(const char *menuText, input_handler handler, void* menuObject);

// Input buffer functions
void reset_input_buffer(UserInputBuffer *inputBuffer);
void append_char(UserInputBuffer *inputBuffer, char c);
uint32_t buffer_length(UserInputBuffer *inputBuffer);
int32_t buffer_to_int(UserInputBuffer *inputBuffer);

#endif
