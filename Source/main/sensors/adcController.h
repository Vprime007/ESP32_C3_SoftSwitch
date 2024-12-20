#ifndef _ADC_CONTROLLER_H
#define _ADC_CONTROLLER_H

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum ADC_Ctrl_Ret_e{
    ADC_CTRL_STATUS_FAIL,
    ADC_CTRL_STATUS_SUCCESS,
}ADC_Ctrl_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
ADC_Ctrl_Ret_t ADC_InitController(void);


#endif//_ADC_CONTROLLER_H