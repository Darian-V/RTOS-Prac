#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/twai.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//GPIO Config
#define TxPin GPIO_NUM_17 //GPIO PINS
#define RxPin GPIO_NUM_18

//TWAI Config
#define TWAIMode TWAI_MODE_NORMAL //For testing change to TWAI_MODE_NO_ACK
static const char *NodeName = "BMS"; //TAG
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TxPin,RxPin,TWAIMode); //Setting TWAI settings using macro initializers
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

//RTOS Task Prio Configuration
#define TX_TSK_PRIO 8
#define RX_TSK_PRIO 9
#define CTRL_TSK_PRIO 10


static const int ID = 0x555;

//Semaphores


//RTOS Tasks
/*static void TWAI_CTRL(void *arg) { //Start & Stop of TWAI Drivers

} */

static void TWAI_Transmit (void *arg) {
    twai_message_t txData;
    txData.identifier = ID;
    txData.data_length_code = 4;
    for (int i = 0; i < 4; i++) {
    txData.data[i] = 0;
    }
    txData.self = 1;

    if (twai_transmit(&txData,portMAX_DELAY) == ESP_OK) {
        ESP_LOGI(NodeName, "Message qued for transmission");
    } else {
        ESP_LOGI(NodeName,"Error: Message not qued for tranmission"); //Add TWAI error code here
    }
    vTaskDelay(pdMS_TO_TICKS(50));
}

static void TWAI_Recieve(void *arg) {
    twai_message_t rxData;

    //Recieve 
    if (twai_receive(&rxData,pdMS_TO_TICKS(1000))== ESP_OK) {
        printf("Messsage Recieved\n");
    } else {
        printf("Failed to recieve message\n");
    }
    ESP_LOGI(NodeName,"Recieved Message: %s", rxData.data);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void app_main(void) {
    //Hardware setup
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(NodeName, "Driver installed");
    } else {
        ESP_LOGI(NodeName,"Failed to install driver");
    }
    
    if (twai_start() == ESP_OK) {
        ESP_LOGI(NodeName,"Driver started\n");
    } else {
        ESP_LOGI(NodeName,"Failed to start driver\n");
    }

    xTaskCreatePinnedToCore(TWAI_Recieve, "BMS_REC",4096,NULL,RX_TSK_PRIO,NULL,tskNO_AFFINITY);
    xTaskCreatePinnedToCore(TWAI_Transmit,"BMS_TRAN",4096,NULL,TX_TSK_PRIO,NULL,tskNO_AFFINITY);
}