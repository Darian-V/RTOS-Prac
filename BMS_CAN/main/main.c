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

u_int8_t data;
u_int8_t ID = 0x555;

//Semaphores


//RTOS Tasks
/*static void TWAI_CTRL(void *arg) { //Start & Stop of TWAI Drivers
    if (twai_start() == ESP_OK) {
        ESP_LOGI(NodeName,"Driver started\n");
    } else {
        ESP_LOGI(NodeName,"Failed to start driver\n");
        return;
    }

} */

static void TWAI_Transmit (void *arg) 
    {
    twai_message_t txData = {.identifier = ID, .data = data, .data_length_code = strlen(data)}
    if (twai_transmit(txData,portMAX_DELAY) == ESP_OK) {
        ESP_LOGI(NodeName, "Message qued for transmission");
        return;
    } else {
        ESP_LOGI(NodeName,"Error: Message not qued for tranmission"); //Add TWAI error code here
        return;
    }
    vtaskDelay(50/pdTICKS_TO_MS);
}

static void TWAI_Recieve(void *arg) {
    twai_message_t rxData;

    //Recieve 
    if (twai_receive(&rx_message,pdMS_TO_Ticks(100))== ESP_OK) {
        printf("Messsage Recieved\n");
    } else {
        printf("Failed to recieve message\n");
        return;
    }

    

}

void app_main(void) {
    //Hardware setup
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(NodeName, "Driver installed");
    } else {
        ESP_LOGI(NodeName,"Failed to install driver");
        return;
    }
}