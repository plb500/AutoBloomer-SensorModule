#include "menu_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


UserInputInt read_int_input(int input, uint16_t defaultValue, UserInputBuffer *inputBuffer) {
    if(input == PICO_ERROR_TIMEOUT) {
        return (UserInputInt) {INPUT_INCOMPLETE, -1};
    }

    char inputChar = (char) (input & 0xFF);

    switch(inputChar) {
        case '\n':
        case '\r':
            {
                // If there is nothing in the input buffer, use the default value
                int32_t argumentValue = defaultValue;
                if(inputBuffer->mCurrentInputPos) {
                    argumentValue = buffer_to_int(inputBuffer);
                }

                return (UserInputInt) {INPUT_COMPLETE, argumentValue};
            }

        case 'q':
        case 'Q':
                return (UserInputInt) {INPUT_ABORTED, -1};

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            printf("%c", inputChar);
            append_char(inputBuffer, inputChar);
            break;

        default:
            break;
    }

    return (UserInputInt) {INPUT_INCOMPLETE, -1};
}

void do_menu(const char *menuText, input_handler handler, void* menuObject) {
    char userInput;
    int printMessage = 1;
    menu_return_behavior menuReturnBehavior = DO_NOTHING;

    // Menu Loop 
    while(menuReturnBehavior != EXIT_MENU){
        if(printMessage) {
            printf(menuText);
            printMessage = 0;

        }

        int c = getchar_timeout_us(100);
        menuReturnBehavior = handler(c, menuObject);
        if(menuReturnBehavior == REPRINT_MENU_TEXT) {
            printMessage = 1;
        }
    }
}

void reset_input_buffer(UserInputBuffer *inputBuffer) {
    if(inputBuffer) {
        inputBuffer->mCurrentInputPos = 0;
        memset(inputBuffer->mBuffer, 0, (MAX_INPUT_LENGTH + 1));
    }
}

void append_char(UserInputBuffer *inputBuffer, char c) {
    if(inputBuffer && inputBuffer->mCurrentInputPos < MAX_INPUT_LENGTH) {
        inputBuffer->mBuffer[inputBuffer->mCurrentInputPos++] = c;
    }
}

uint32_t buffer_length(UserInputBuffer *inputBuffer) {
    return inputBuffer ? inputBuffer->mCurrentInputPos : 0;
}

int32_t buffer_to_int(UserInputBuffer *inputBuffer) {
    if(inputBuffer) {
        // Ensure buffer is null-terminated
        int32_t zerosToPad = ((MAX_INPUT_LENGTH + 1) - inputBuffer->mCurrentInputPos);
        memset(inputBuffer->mBuffer + inputBuffer->mCurrentInputPos, 0, zerosToPad);

        return atoi(inputBuffer->mBuffer);
    }

    return 0;
}
