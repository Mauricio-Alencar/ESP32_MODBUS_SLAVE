/*
 * Autor: Maurício Alencar
 * Exemplo de codigo main para uso da biblioteca Modbus RTU MASTER
 * Site: coldpack.com.br
 *
 * Biblioteca Modbus RTU: MASTER
 * Copyright (C) 2019 Coldpack Mechatronics Systems <mauricioalencarf@gmail.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
*/

/******************************* LIBRARYS *****************************/
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
#include "driver/gpio.h"
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

/******************************* DEFINES *****************************/
static const char* TAG = "MB_SLAVE_MAIN";

#define DEBUG_ESP32     TRUE

volatile UCHAR MB_SLAVE_ADDRESS = 0X01;

/******************************* PROTOTIPOS *****************************/
void mb_task(void *pvParameters); 

/******************************* CALLBACK FUNCTIONS MODBUS *****************************/
// ----------------------------------------------- CALL BACK FUNC CODE 01 ---------------------------------------------
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

// ----------------------------------------------- CALL BACK FUNC CODE 03 ---------------------------------------------
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
/*
BOOL MBEventReadRegisters(USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters  )
{
    UCHAR i = 0;
    static USHORT contador1 = 0;
    static USHORT contador2 = 1000;

    if (DEBUG_ESP32) ESP_LOGI(TAG, "MB_READREGISTER FUNCTION");
    if (DEBUG_ESP32) ESP_LOGI(TAG, "START ADDRES: '%d'", usStartAddress);
    if (DEBUG_ESP32) ESP_LOGI(TAG, "NUMBER OF registers: '%d'", usNumberOfRegisters);

    switch(usStartAddress)
    {
       case 5: contador1++;
                wRegHoldingbuffer[i++] = (UCHAR) (contador1);
                wRegHoldingbuffer[i++] = (UCHAR) (contador1>>8);
                
                //break;
                
       case 6: contador2++; 

                wRegHoldingbuffer[i++] = (UCHAR) (contador2);
                wRegHoldingbuffer[i++] = (UCHAR) (contador2>>8);
                break;
    }
    return TRUE;
}
*/

// ----------------------------------------------- CALL BACK FUNC CODE 06 ---------------------------------------------
/**
 * @brief      { Function 06 Modbus RTU RS485 Slave }
 *
 * 
 * @param  usStartAddress   The start address
 * @param  usValue  The value of the registers to master write
 *
 * @return     { description_of_the_return_value }
 */
//descomentar função abaixo para utilizar-la
/*
#define RELE    5
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<RELE) 
BOOL MBEventWriteSingleRegister( USHORT usStartAddress, USHORT  usValue )
{

    if (DEBUG_ESP32) ESP_LOGI(TAG, "MB_SingleWriteREG FUNCTION");
    if (DEBUG_ESP32) ESP_LOGI(TAG, "START ADDRES: '%d'", usStartAddress);
    if (DEBUG_ESP32) ESP_LOGI(TAG, "VALUE: '%d'", usValue);


    if(usStartAddress == 1)
    {
        gpio_config_t io_conf;
        //disable interrupt
        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        //set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        //bit mask para pino 5
        io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
        //disable pull-down mode
        io_conf.pull_down_en = 0;
        //disable pull-up mode
        io_conf.pull_up_en = 0;
        //configure GPIO with the given settings
        gpio_config(&io_conf);

        
        //Caso o valor do registrador 1 seja 1001, liga RELE1, caso contrário
        //desliga;
        
        gpio_set_level(RELE, ((usValue == 1001)?1:0));  
    }

    return TRUE;
    
}
*/

// ----------------------------------------------- CALL BACK FUNC CODE 16 ---------------------------------------------
/**
 * @brief      { Function 16 Write Multiple Registers - ModbusRTU Slave}
 *
 * @param[in]  usStartAddress       The start address
 * @param      wRegHoldingbuffer    The w register holdingbuffer
 * @param[in]  usNumberOfRegisters  The number of registers
 *
 * @return     { true }
 */
//descomentar função abaixo para utilizar-la

#define RELE    5
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<RELE)
BOOL MBEventWriteMultipleRegisters( USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters )
{
    USHORT value;
    USHORT i;

    if (DEBUG_ESP32) ESP_LOGI(TAG, "MB_MultipleWriteREG FUNCTION");
    if (DEBUG_ESP32) ESP_LOGI(TAG, "START ADDRES: '%d'", usStartAddress);
    if (DEBUG_ESP32) ESP_LOGI(TAG, "NUMBER OF registers: '%d'", usNumberOfRegisters);

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask para pino 5
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    for( i = usStartAddress ; i < usStartAddress + usNumberOfRegisters; ++i)
    {
         if(i == 1)
         {
            if (DEBUG_ESP32) ESP_LOGI(TAG, "FUNC 16 REG 1");

            value = (wRegHoldingbuffer[0] << 8) | wRegHoldingbuffer[1];
            /**
             * Caso o valor do registrador 1 seja 1001, liga RELE1, caso contrário
             * desliga;
             */
            gpio_set_level(RELE, ((value == 1001)?1:0));  
         }

         if(i == 2)
         {
            if (DEBUG_ESP32) ESP_LOGI(TAG, "FUNC 16 REG 2");
         }
    }
    return TRUE;
}

// -------------------------------------------- MAIN --------------------------------------------------------
void app_main()
{
	MBInit();
    xTaskCreate(mb_task, "mb_task", 1024*4, NULL, 2, NULL);
} 
// ------------------------------------------- FIM MAIN ------------------------------------------------------

// -------------------------------------------- TAREFAS ------------------------------------------------------
void mb_task(void *pvParameters)
{
    while(1)
    {
        MB();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
// ------------------------------------------ FIM TAREFAS ------------------------------------------------------

// -------------------------------------------- EXCEPTION ------------------------------------------------------
void MBException( USHORT usExceptionCode ) {
   return; 
}
// --------------------------------------- FIM EXCEPTION -------------------------------------------------------

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


BOOL MBEventReadRegisters(USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters  )
{
   return TRUE;
}


BOOL MBEventWriteSingleRegister( USHORT usStartAddress, USHORT  usValue )
{
   return TRUE;
}

/*
BOOL MBEventWriteMultipleRegisters( USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters )
{
  return TRUE;
}
*/
