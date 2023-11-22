#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/twai.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//GPIO Config
#define TxPin GPIO_NUM_17 //CAN TX
#define RxPin GPIO_NUM_18 // CAN RX

//TWAI Config
#define TWAIMode TWAI_MODE_NO_ACK //For testing change to TWAI_MODE_NO_ACK
static const char *TWAI = "CAN_LOG:"; 
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TxPin,RxPin,TWAIMode); //Setting TWAI settings using macro initializers
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

//System Config:
static const char *SYS_LOG = "SYS_LOG:"; 
#define TX_TSK_PRIO 8 //Task Priority
#define RX_TSK_PRIO 9
#define CTRL_TSK_PRIO 10
#define BAT_CTRL_PRIO 25

//Control Flow Globals:
typedef struct { //System Instruction Structure
    uint8_t id; //Should limit to an 8 byte ID?
    char inst[8]; //System instructions up to 8 bytes 
} SysInst; 
//RTOS Tasks

static void BAT_CTRL(void *arg) { //Battery Control Task: 
    SysInst BMS_CTRL; //BMS Instruction Struct 
    QueueHandle_t SystemCTRL;
    SystemCTRL = xQueueCreate(10,sizeof(BMS_CTRL)); 
    if (SystemCTRL == 0) {
        ESP_LOGI(SYS_LOG, "System Instruction Queue was not created ")
    }
    while (1) {
            //Add MOSFET/Bat Cell Control flow here (TO BE DECIDED)
    }
} 

static void TWAI_CTRL(void *arg) {

}

static void TWAI_Transmit (void *arg) {
    QueueHandle_t TWAI_Transmit_queue;
    TWAI_Transmit_queue = xQueueCreate(5, sizeof(twai_message_t)); //May neec to fix sizeof(twai_message_t) ***
    if (TWAI_Transmit_queue == 0) {
        ESP_LOGI(SYS_LOG,"TWAI Transmit Queue was not created");
    }
    SysInst outData;
    while(1) {
    if (xQueueReceive(TWAI_Transmit_queue, (void*)&outData,0) == pdTRUE) { //Check que for message to be sent. 
        twai_message_t txData; //Message structure init
        txData.identifier = outData.id; 
        txData.data_length_code = strlen(outData.inst);
        for (i = 0, i < txData.data_length_code, i ++) { //Copy string into structure
            txData.data[i] = outData.inst[i];
        }
        txData.self = 0;
    } 
    
    if (twai_transmit(&txData,portMAX_DELAY) == ESP_OK) {
        ESP_LOGI(TWAI, "Message qued for transmission");
    } else {
        ESP_LOGI(TWAI,"Error: Message not qued for tranmission"); //Add TWAI error code here
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    }
    
}

static void TWAI_Recieve(void *arg) {
    

    //Recieve 
    while (1) {
    twai_message_t rxData;
    if (twai_receive(&rxData,pdMS_TO_TICKS(1000))== ESP_OK) {
        printf("Messsage Recieved\n");
    } else {
        printf("Failed to recieve message\n");
    }
    ESP_LOGI(TWAI,"Recieved Message: %s", rxData.data);
    vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

void app_main(void) {
    //Hardware setup
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TWAI, "Driver installed");
    } else {
        ESP_LOGI(TWAI,"Failed to install driver");
    }
    
    if (twai_start() == ESP_OK) {
        ESP_LOGI(TWAI,"Driver started\n");
    } else {
        ESP_LOGI(TWAI,"Failed to start driver\n");
    }

    xTaskCreatePinnedToCore(TWAI_Recieve, "BMS_REC",4096,NULL,RX_TSK_PRIO,NULL,tskNO_AFFINITY);
    xTaskCreatePinnedToCore(TWAI_Transmit,"BMS_TRAN",4096,NULL,TX_TSK_PRIO,NULL,tskNO_AFFINITY);
    xTaskCreatePinnedToCore(BAT_CTRL, "Battery_Contol", 4096, NULL, BAT_CTRL_PRIO,tskNO_AFFINITY);
}     