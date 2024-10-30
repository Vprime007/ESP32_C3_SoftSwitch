#ifndef _BUTTON_CONTROLLER_H
#define _BUTTON_CONTROLLER_H

#include <stdint.h>

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define BTN_MAX_NUMBER_OF_BUTTON                (8)

typedef void(*btnPressedCallback)(void);
typedef void(*btnReleasedCallback)(void);

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum BTN_Active_Level_e{
    BTN_ACTIVE_LEVEL_LOW,
    BTN_ACTIVE_LEVEL_HIGH,

    BTN_ACTIVE_LEVEL_INVALID,
}BTN_Active_Level_t;

typedef enum BTN_Ctrl_Ret_e{
    BTN_CTRL_STATUS_FAIL,
    BTN_CTRL_STATUS_SUCCESS,
}BTN_Ctrl_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
BTN_Ctrl_Ret_t BTN_InitController(void);

BTN_Ctrl_Ret_t BTN_AddButton(uint8_t io_num,
                             BTN_Active_Level_t active_level,
                             btnPressedCallback pressed_callback,
                             btnReleasedCallback released_callback);

#endif//_BUTTON_CONTROLLER_H