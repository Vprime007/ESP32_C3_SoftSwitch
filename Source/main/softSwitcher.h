#ifndef _SOFT_SWITCHER_H
#define _SOFT_SWITCHER_H

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum SOFT_IO_Id_e{
    SOFT_SWITCHER_PWR_ID,
    SOFT_SWITCHER_CHARGING_ID,

    SOFT_SWITCHER_INVALID_ID,
}SOFT_IO_Id_t;

typedef enum SOFT_IO_Level_e{
    SOFT_IO_LEVEL_LOW,
    SOFT_IO_LEVEL_HIGH,
}SOFT_IO_Level_t;

typedef struct SOFT_IO_Config_e{
    uint8_t io_num;
    SOFT_IO_Level_t active_level; 
}SOFT_IO_Config_t;

typedef enum SOFT_Switcher_Ret_e{
    SOFT_SWITCHER_STATUS_FAIL,
    SOFT_SWITCHER_STATUS_SUCCESS,
}SOFT_Switcher_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Soft Switcher module initialization
*
*   This function is used to initialize the Soft switcher.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pwr_io              Power output config
*   \param[in]  charging_io         Charging output config
*
*   \return     operation status
*
*******************************************************************************/
SOFT_Switcher_Ret_t SOFT_InitModule(SOFT_IO_Config_t pwr_io, SOFT_IO_Config_t charging_io);

/***************************************************************************//*!
*  \brief Soft Switcher set output ON
*
*   This function is used to turn ON an output
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  io_id               IO id to turn ON
*
*   \return     operation status
*
*******************************************************************************/
SOFT_Switcher_Ret_t SOFT_SetOutput(SOFT_IO_Id_t io_id);

/***************************************************************************//*!
*  \brief Soft Switcher set output OFF
*
*   This function is used to turn OFF an output
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  io_id               IO id to turn OFF
*
*   \return     operation status
*
*******************************************************************************/
SOFT_Switcher_Ret_t SOFT_ClearOutput(SOFT_IO_Id_t io_id);

/***************************************************************************//*!
*  \brief Soft Switcher get output state (ON/OFF)
*
*   This function is used to get an output state.
*   return 1 if output ON / return 0 if output OFF.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  io_id               IO id to turn OFF
*   \param[out] pLevel              Pointer to store IO state
*
*   \return     operation status
*
*******************************************************************************/
SOFT_Switcher_Ret_t SOFT_GetIOState(SOFT_IO_Id_t io_id, uint8_t *pLevel);

#endif//_SOFT_SWITCHER_H