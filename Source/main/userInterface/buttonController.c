/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdbool.h>

#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "buttonController.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define BUTTON_MONITORING_PERIOD_MS     (10)
#define NB_SAMPLE_FOR_PRESSED           (5)
#define NB_SAMPLE_FOR_RELEASED          (5)

#define LOG_LOCAL_LEVEL                 (ESP_LOG_INFO)

/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/
typedef enum Button_State_e{
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESSED,
}Button_State_t;

typedef struct Button_s{
    uint8_t io;
    Button_State_t prev_state;
    BTN_Active_Level_t active_level;
    uint16_t debounce_cptr;
    btnPressedCallback pressed_callback;
    btnReleasedCallback released_callback;
}Button_t;

/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static bool isTableFull(void);
static void addButtonToTable(Button_t *pButton_config);

static void tButtonTask(void *pvParameters);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static Button_t button_table[BTN_MAX_NUMBER_OF_BUTTON];

static TaskHandle_t button_task_handle = NULL;
static SemaphoreHandle_t button_mutex_handle = NULL;

static const char * TAG = "BTN";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static void tButtonTask(void *pvPatameters){

    ESP_LOGI(TAG, "Starting Button task");

    for(;;){

        for(uint8_t i=0; i<BTN_MAX_NUMBER_OF_BUTTON; i++){
            
            if(button_table[i].io != 0xFF){

                Button_State_t state = BUTTON_STATE_RELEASED;

                if((gpio_get_level(button_table[i].io) && (button_table[i].active_level == BTN_ACTIVE_LEVEL_HIGH))||
                   (!gpio_get_level(button_table[i].io) && (button_table[i].active_level == BTN_ACTIVE_LEVEL_LOW))){
                    //button currently pressed
                    state = BUTTON_STATE_PRESSED;
                }

                if(state != button_table[i].prev_state){
                    button_table[i].prev_state = state;
                    button_table[i].debounce_cptr = 0;
                }
                else{

                    if(state == BUTTON_STATE_PRESSED){
                        if((button_table[i].debounce_cptr >= NB_SAMPLE_FOR_PRESSED) && 
                           (button_table[i].debounce_cptr != 0xFFFF)){

                            button_table[i].debounce_cptr = 0xFFFF;
                            if(button_table[i].pressed_callback != NULL)    button_table[i].pressed_callback();
                        }
                    }
                    else{
                        if((button_table[i].debounce_cptr >= NB_SAMPLE_FOR_RELEASED) && 
                           (button_table[i].debounce_cptr != 0xFFFF)){

                            button_table[i].debounce_cptr = 0xFFFF;
                            if(button_table[i].released_callback != NULL)   button_table[i].released_callback();
                        }
                    }

                    if(button_table[i].debounce_cptr != 0xFFFFF)    button_table[i].debounce_cptr++;
                }
            }
        }

        vTaskDelay(BUTTON_MONITORING_PERIOD_MS/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static bool isTableFull(void){

    bool available_space = true;

    for(uint8_t i=0; i<BTN_MAX_NUMBER_OF_BUTTON; i++){
        if(button_table[i].io == 0xFF){
            //Space available
            available_space = false;
            break;
        }
    }

    return available_space;
}

static void addButtonToTable(Button_t *pButton_config){

    //Scan the button table for the first available index
    uint8_t index = 0;
    for(index = 0; index<BTN_MAX_NUMBER_OF_BUTTON; index++){
        //Check if table index is empty
        if(button_table[index].io == 0xFF){
            //Register button in available index
            button_table[index].io = pButton_config->io;
            button_table[index].prev_state = BUTTON_STATE_RELEASED;
            button_table[index].active_level = pButton_config->active_level;
            button_table[index].pressed_callback = pButton_config->pressed_callback;
            button_table[index].released_callback = pButton_config->released_callback;
            button_table[index].debounce_cptr = 0;
            break;
        }
    }

    ESP_LOGI(TAG, "Failed to store button in table");
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
BTN_Ctrl_Ret_t BTN_InitController(void){

    //Init button table
    for(uint8_t i=0; i<BTN_MAX_NUMBER_OF_BUTTON; i++){
        button_table[i].active_level = BTN_ACTIVE_LEVEL_INVALID;
        button_table[i].debounce_cptr = 0;
        button_table[i].io = 0xFF;
        button_table[i].prev_state = BUTTON_STATE_RELEASED;
        button_table[i].pressed_callback = NULL;
        button_table[i].released_callback = NULL;
    }

    //Create button mutex
    button_mutex_handle = xSemaphoreCreateMutex();
    if(button_mutex_handle == NULL){
        ESP_LOGI(TAG, "Failed to create button mutex");
        return BTN_CTRL_STATUS_FAIL;
    }

    //Create button task
    if(pdTRUE == xTaskCreate(tButtonTask,
                             "Button Task",
                             2048,
                             NULL,
                             5,
                             &button_task_handle)){
        ESP_LOGE(TAG, "Failed to create button taks");
        return BTN_CTRL_STATUS_FAIL;
    }

    return BTN_CTRL_STATUS_SUCCESS;
}

BTN_Ctrl_Ret_t BTN_AddButton(uint8_t io_num,
                             BTN_Active_Level_t active_level,
                             btnPressedCallback pressed_callback,
                             btnReleasedCallback released_callback){

    if(active_level >= BTN_ACTIVE_LEVEL_INVALID){
        ESP_LOGE(TAG, "Failed to add button: Invalid param.");
        return BTN_CTRL_STATUS_FAIL;
    }

    xSemaphoreTake(button_mutex_handle, portMAX_DELAY);

    if(!isTableFull()){
        //Store new button in table
        Button_t btn_config = {
            .io = io_num,
            .active_level = active_level,
            .pressed_callback = pressed_callback,
            .released_callback = released_callback,
        };

        addButtonToTable(&btn_config);
    }

    xSemaphoreGive(button_mutex_handle);

    return BTN_CTRL_STATUS_SUCCESS;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/


