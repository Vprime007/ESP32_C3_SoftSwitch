/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
/******************************************************************************
*   Includes
*******************************************************************************/
#include "freertos/freeRTOS.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "softSwitcher.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define LOG_LOCAL_LEVEL                 (ESP_LOG_INFO)

/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/


/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/


/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static uint8_t pwr_io_num = 0xFF;
static SOFT_IO_Level_t pwr_active_level = SOFT_IO_LEVEL_HIGH;

static uint8_t charging_io_num = 0xFF;
static SOFT_IO_Level_t charging_active_level = SOFT_IO_LEVEL_HIGH;

static SemaphoreHandle_t soft_mutex_handle = NULL;

static const char * TAG = "SOFT_SWITCHER";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
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
SOFT_Switcher_Ret_t SOFT_InitModule(SOFT_IO_Config_t pwr_io, SOFT_IO_Config_t charging_io){

    //Create mutex
    soft_mutex_handle = xSemaphoreCreateMutex();
    if(soft_mutex_handle == NULL){
        ESP_LOGW(TAG, "Failed to create Soft Switcher mutex");
        return SOFT_SWITCHER_STATUS_FAIL;
    }

    xSemaphoreTake(soft_mutex_handle, portMAX_DELAY);

    pwr_io_num = pwr_io.io_num;
    pwr_active_level = pwr_io.active_level;

    charging_io_num = charging_io.io_num;
    charging_active_level = charging_io.active_level; 

    gpio_config_t cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = ((1ULL<<pwr_io.io_num) | (1uLL<<charging_io.io_num)),
    };
    gpio_config(&cfg);

    //Set IOs to low level
    if(pwr_active_level == SOFT_IO_LEVEL_HIGH){
        gpio_set_level(pwr_io_num, 0);
    }
    else{
        gpio_set_level(pwr_io_num, 1);
    }

    if(charging_active_level == SOFT_IO_LEVEL_HIGH){
        gpio_set_level(charging_io_num, 0);
    }
    else{
        gpio_set_level(charging_io_num, 1);
    }

    xSemaphoreGive(soft_mutex_handle);

    return SOFT_SWITCHER_STATUS_SUCCESS;
}

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
SOFT_Switcher_Ret_t SOFT_SetOutput(SOFT_IO_Id_t io_id){

    if(io_id >= SOFT_SWITCHER_INVALID_ID){
        ESP_LOGW(TAG, "Failed to set output -> Invalid ID");
        return SOFT_SWITCHER_STATUS_FAIL;
    }

    xSemaphoreTake(soft_mutex_handle, portMAX_DELAY);

    switch(io_id){
        case SOFT_SWITCHER_PWR_ID:
        {
            gpio_set_level(pwr_io_num, (pwr_io_num == SOFT_IO_LEVEL_HIGH));
        }
        break;
        
        case SOFT_SWITCHER_CHARGING_ID:
        {
            gpio_set_level(charging_io_num, (charging_active_level == SOFT_IO_LEVEL_HIGH));
        }
        break;

        default:
        {
            //Do nothing...
        }
        break;
    }

    xSemaphoreGive(soft_mutex_handle);

    return SOFT_SWITCHER_STATUS_SUCCESS;
}

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
SOFT_Switcher_Ret_t SOFT_ClearOutput(SOFT_IO_Id_t io_id){

    if(io_id >= SOFT_SWITCHER_INVALID_ID){
        ESP_LOGW(TAG, "Failed to clear output -> Invalid ID");
        return SOFT_SWITCHER_STATUS_FAIL;
    }

    xSemaphoreTake(soft_mutex_handle, portMAX_DELAY);

    switch(io_id){
        case SOFT_SWITCHER_PWR_ID:
        {
            gpio_set_level(pwr_io_num, (pwr_active_level != SOFT_IO_LEVEL_HIGH));
        }
        break;
        
        case SOFT_SWITCHER_CHARGING_ID:
        {
            gpio_set_level(charging_io_num, (charging_active_level != SOFT_IO_LEVEL_HIGH));
        }
        break;

        default:
        {
            //Do nothing...
        }
        break;
    }

    xSemaphoreGive(soft_mutex_handle);

    return SOFT_SWITCHER_STATUS_SUCCESS;
}

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
SOFT_Switcher_Ret_t SOFT_GetIOState(SOFT_IO_Id_t io_id, uint8_t *pLevel){

    if(io_id >= SOFT_SWITCHER_INVALID_ID){
        ESP_LOGW(TAG, "Failed to get output -> Invalid ID");
        return SOFT_SWITCHER_STATUS_FAIL;
    }

    uint8_t current_level = 0;
    SOFT_IO_Level_t active_level = SOFT_IO_LEVEL_HIGH;

    xSemaphoreTake(soft_mutex_handle, portMAX_DELAY);

    switch(io_id){
        case SOFT_SWITCHER_PWR_ID:
        {
            active_level = pwr_active_level;
            current_level = gpio_get_level(pwr_io_num);
        }
        break;

        case SOFT_SWITCHER_CHARGING_ID:
        {
            active_level = charging_active_level;
            current_level = gpio_get_level(charging_io_num);
        }
        break;

        default:
        {
            //Do nothing...
        }
        break;
    }   

    xSemaphoreGive(soft_mutex_handle);

    if(((current_level == 1) && (active_level == SOFT_IO_LEVEL_HIGH)) || 
       ((current_level == 0) && (active_level == SOFT_IO_LEVEL_LOW))){

        if(pLevel != NULL)  *pLevel = 1;
    }
    else{
        if(pLevel != NULL)  *pLevel = 0;
    }

    return SOFT_SWITCHER_STATUS_SUCCESS;
}


/******************************************************************************
*   Interrupts
*******************************************************************************/


