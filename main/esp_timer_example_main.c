/* esp_timer (high resolution timer) example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "cJSON.h"

#include "ESP32_ModbusRTU_Slave.h"

static const char* TAG = "example";
volatile UCHAR MB_SLAVE_ADDRESS = 0X01;
void mb_task(void *pvParameters); 
void rx_task(void *arg);

UCHAR mb_buffer[MB_BUFFER_SIZE];
UCHAR mb_buffer_indice = 0;
uint8_t data[1] = {0};

/**
 * @brief      { Function 01 Modbus RTU RS485 Slave }
 *
 * @param      ucCoils          The uc coils
 * @param[in]  usStartAddress   The start address
 * @param[in]  usNumberOfCoils  The number of coils
 *
 * @return     { description_of_the_return_value }
 */
BOOL MBEventReadCoils(USHORT usStartAddress, UCHAR * ucCoils, USHORT  usNumberOfCoils )
{  
    switch( usStartAddress )
    {
        case 1:
               if( usNumberOfCoils > 0 ) {              
                  ucCoils[0] = 0B10101010;
               }
               break;
    }

    return TRUE;
}



void app_main()
{
	MBInit();
	xTaskCreate(rx_task, "uart_rx_task", 1024*4, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(mb_task, "mb_task", 1024*4, NULL, 2, NULL);
} 

void mb_task(void *pvParameters)
{
    while(1)
    {
        MB();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void rx_task(void *arg)
{

    ESP_LOGI(TAG, "UART TASK CRIADA!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    while(1) 
    {
      	//int len = uart_read_bytes(UART_NUM_1, data, 1, portMAX_DELAY);
    	int len = uart_read_bytes(UART_NUM_1, data, 1, 500/portTICK_PERIOD_MS);

      	if(len > 0)
      	{

		    mb_buffer[mb_buffer_indice] = data[0];
		    mb_buffer_indice++;

		    if(mb_buffer_indice >= MB_BUFFER_SIZE)
		            mb_buffer_indice = 0;

		    ESP_LOGI(TAG, "Read: '%.2X'", mb_buffer[mb_buffer_indice-1]);
		    MBTimerRestart();
		}
    }
    vTaskDelete(NULL);
}



/*

 Callback das Funções do Stack Modbus; 
 Para Habilitar ou desabilitar cada uma das funções individualmente, configure
 o arquivo mb_config.h em "C:\...\Documents\Arduino\libraries\ModbusRTU-RS485-Master\src".
 Caso as funções estejam habilitadas, mantenha a função de callback em seu programa.

*/

void MBException( USHORT usExceptionCode ) {
   return; 
}


BOOL MBEventReadRegisters(USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters  )
{
   return TRUE;
}

BOOL MBEventWriteSingleRegister( USHORT usStartAddress, USHORT  usValue )
{
   return TRUE;
}
BOOL MBEventWriteMultipleRegisters( USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters )
{
  return TRUE;
}

