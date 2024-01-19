#include "user_int_input_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"

const uint32_t NO_USER_INPUT_VALUE = 0xFFFFFFFF;


menu_return_behavior handle_user_input(int input, UserIntInputMenuObject *menuObject) {
    UserInputInt userResponse = read_int_input(input, menuObject->mDefaultValue, &menuObject->mInputStateHolder.mArgumentBuffer);
    menu_return_behavior returnBehavior = DO_NOTHING;

    switch(userResponse.mInputState) {
        case INPUT_COMPLETE:
            if((userResponse.mIntValue >= menuObject->mMinValue) && (userResponse.mIntValue <= menuObject->mMaxValue)) {
                printf("\n");
                menuObject->mEnteredValue = userResponse.mIntValue;
                menuObject->mInputStateHolder.mState = INPUT_COMPLETED;
            } else {
                printf("\nInvalid value (%d). Value must be within the range %d-%d\n", 
                    userResponse.mIntValue,
                    menuObject->mMinValue, 
                    menuObject->mMaxValue
                );
                menuObject->mInputStateHolder.mState = PROMPT_USER;
            }
            break;
    
        case INPUT_ABORTED:
            printf("\n");
            menuObject->mEnteredValue = NO_USER_INPUT_VALUE;
            returnBehavior = EXIT_MENU;

        case INPUT_INCOMPLETE:
        default:
            break;
    }

    return returnBehavior;
}

menu_return_behavior user_int_input_menu_handler(int input, void *menuObject) {
    UserIntInputMenuObject* userIntInputMenuObject = (UserIntInputMenuObject*) menuObject;
    if(!userIntInputMenuObject) {
        return EXIT_MENU;
    }

    switch(userIntInputMenuObject->mInputStateHolder.mState) {
        case PROMPT_USER:
            printf(userIntInputMenuObject->mUserPrompt);
            reset_input_buffer(&userIntInputMenuObject->mInputStateHolder.mArgumentBuffer);
            userIntInputMenuObject->mInputStateHolder.mState = READING_INPUT;
            break;
        case READING_INPUT:
            return handle_user_input(input, userIntInputMenuObject);
        case INPUT_COMPLETED:
            return EXIT_MENU;
        default:
            break;
    }

    return DO_NOTHING;
}

void reset_user_int_input_menu_object(UserIntInputMenuObject* menuObject) {
    menuObject->mInputStateHolder.mState = PROMPT_USER;
    reset_input_buffer(&(menuObject->mInputStateHolder.mArgumentBuffer));
}

