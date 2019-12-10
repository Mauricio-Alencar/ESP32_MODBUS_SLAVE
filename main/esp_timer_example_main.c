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
//#include "esp_event_loop.h"
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

static const char* TAG = "MB_SAMPLE_COIL_REGISTER";

#define DEBUG_ESP32     TRUE

volatile UCHAR MB_SLAVE_ADDRESS = 0X01;
void mb_task(void *pvParameters); 



/**
 * @brief      { Function 01 Modbus RTU RS485 Slave }
 *
 * @param      ucCoils          The uc coils
 * @param[in]  usStartAddress   The start address
 * @param[in]  usNumberOfCoils  The number of coils
 *
 * @return     { description_of_the_return_value }
 * 
 */
//descomentar função abaixo para utilizar-la
/* 
BOOL MBEventReadCoils(USHORT usStartAddress, UCHAR * ucCoils, USHORT  usNumberOfCoils )
{  
    if (DEBUG_ESP32) ESP_LOGI(TAG, "MB_READCOIL FUNCTION");
    if (DEBUG_ESP32) ESP_LOGI(TAG, "START ADDRES: '%d'", usStartAddress);
    if (DEBUG_ESP32) ESP_LOGI(TAG, "NUMBER OF COILS: '%d'", usNumberOfCoils);

    switch( usStartAddress )
    {
        case 0:
               if( usNumberOfCoils > 0 ) {              
                  ucCoils[0] = 0B01010101;
                  ucCoils[1] = 0B00000001;
               }
               break;
        case 1:
               if( usNumberOfCoils > 0 ) { 
                  ucCoils[0] = 0B11110000;
                  ucCoils[1] = 0B00000001;
               }
               break;
            default:
            ESP_LOGI(TAG, "DEFAULT!");
            ucCoils[0] = 0B11111111;
            ucCoils[1] = 0B11111111;
            ucCoils[2] = 0B11111111;
            ucCoils[3] = 0B11111111;
            break;
    }

    return TRUE;
}
*/

/**
 * @brief      { Function 03 Modbus RTU RS485 Slave }
 *
 * @param      wRegHoldingbuffer    buffer of hold registers
 * @param[in]  usStartAddress   The start address
 * @param[in]  usNumberOfRegisters  The number of registers
 *
 * @return     { description_of_the_return_value }
 */
//descomentar função abaixo para utilizar-la
BOOL MBEventReadRegisters(USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters  )
{
    UCHAR i = 0;
    USHORT contador1 = 0;
    USHORT contador2 = 1000;

    switch(usStartAddress)
    {
       case 5: contador1++;
                wRegHoldingbuffer[i++] = (UCHAR) (contador1>>8);
                wRegHoldingbuffer[i++] = (UCHAR) (contador1);
                break;
                
       case 6: contador2++; 
                wRegHoldingbuffer[i++] = (UCHAR) (contador2>>8);
                wRegHoldingbuffer[i++] = (UCHAR) (contador2);
                break;
    }
    return TRUE;
}

void app_main()
{
	MBInit();
    xTaskCreate(mb_task, "mb_task", 1024*4, NULL, 2, NULL);
} 

void mb_task(void *pvParameters)
{
    while(1)
    {
        MB();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void MBException( USHORT usExceptionCode ) {
   return; 
}

/*

 Callback das Funções do Stack Modbus; 
 Para Habilitar ou desabilitar cada uma das funções individualmente, configure
 o arquivo mb_config.h.
 Caso as funções estejam habilitadas, mantenha a função de callback em seu programa e comentar apenas a que estiver usando acima.

*/

BOOL MBEventReadCoils(USHORT usStartAddress, UCHAR * ucCoils, USHORT  usNumberOfCoils )
{  
    return TRUE;
}

/*
BOOL MBEventReadRegisters(USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters  )
{
   return TRUE;
}
*/

BOOL MBEventWriteSingleRegister( USHORT usStartAddress, USHORT  usValue )
{
   return TRUE;
}
BOOL MBEventWriteMultipleRegisters( USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters )
{
  return TRUE;
}

