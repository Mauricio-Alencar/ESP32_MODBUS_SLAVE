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

#include "mb_config.h"
#include "mb_serial.h"
#include "mb_timer.h"
#include "mb_event.h"

//Configuraçãop da uart em 9600 BPS
#define BAUD 9600
#define TXD_PIN (4)
#define RXD_PIN (16)
#define RTS_PIN (27)

//uart num define
#if defined UART1
#define EX_UART_NUM UART_NUM_1
#elif defined UART2
#define EX_UART_NUM UART_NUM_2 
#endif

static const char *TAG = "MB_SERIAL";

void rx_task(void *arg);

/* ----------------------- Variables ----------------------------------------*/
static const int RX_BUF_SIZE = 256;

EventGroupHandle_t RX_event_group;
const int RX_BIT = BIT0;

UCHAR mb_buffer[MB_BUFFER_SIZE];
UCHAR mb_buffer_indice = 0;
uint8_t data[1] = {0};

//vetores de interrupção da UART, em caso de uso para RTOS, tarefa com alimentação na recepção recepção serial.
void rx_task(void *arg)
{

    ESP_LOGI(TAG, "UART TASK CRIADA!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    while(1) 
    {
        //Espera indefinidamente enquanto RX_BIT estiver bloqueando o uso da task
        xEventGroupWaitBits(RX_event_group, RX_BIT, false, false, portMAX_DELAY);

      	//int len = uart_read_bytes(UART_NUM_1, data, 1, portMAX_DELAY);
    	int len = uart_read_bytes(UART_NUM_1, data, 1, 500/portTICK_PERIOD_MS);

      	if(len > 0)
      	{

		    mb_buffer[mb_buffer_indice++] = data[0];

		    if(mb_buffer_indice >= MB_BUFFER_SIZE)
		            mb_buffer_indice = 0;

		    ESP_LOGI(TAG, "Read: '%.2X'", mb_buffer[mb_buffer_indice-1]);
		    MBTimerRestart();
		}
    }
    vTaskDelete(NULL);
}

//função para inicialização da serial
BOOL
MBUartInit( void )
{ 
#if defined UART1
	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);

    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, RTS_PIN, UART_PIN_NO_CHANGE);
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE, 0, 0, NULL, 0);

#elif defined UART2 
	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_2, &uart_config);

    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, RTS_PIN, UART_PIN_NO_CHANGE);
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE, 0, 0, NULL, 0);

#endif   
    RX_event_group = xEventGroupCreate();
    xEventGroupSetBits(RX_event_group, RX_BIT);

	xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    return TRUE;
} 

//função para habilitação da recepção serial
void 
MBUartRXEnable( void )
{   
	ESP_LOGI(TAG,"ATIVANDO SERIAL!");
    xEventGroupSetBits(RX_event_group, RX_BIT);
	MBReceive_On_RS485(); 
}

//função para desabilitar a recepção serial
void 
MBUartRXDisable( void )
{
	ESP_LOGI(TAG,"DESATIVANDO SERIAL!");
    xEventGroupClearBits(RX_event_group, RX_BIT); 
	MBTransmit_On_RS485();
}

//função para enviar um caracterer no barramento rs485
void
MBUartRxSend( const char ch )
{
#if defined UART1
    //aguarda o envio de pacotes que estao sendo processados na serial.
    ESP_ERROR_CHECK(uart_wait_tx_done(EX_UART_NUM, 100));
    uart_write_bytes (EX_UART_NUM, &ch, 1); 
    
#elif defined UART2 
    ESP_ERROR_CHECK(uart_wait_tx_done(EX_UART_NUM, 100));
    uart_write_bytes (EX_UART_NUM, &ch, 1);
#endif      			
}

//função para habilitar a interrupção da serial
void 
MBUartIE( void )
{
  
}

//função para controle de SET no pino RTS do barramento rs485
void 
MBTransmit_On_RS485( void )
{
    uart_set_rts(EX_UART_NUM, TRUE); 
    vTaskDelay(5 / portTICK_PERIOD_MS);
}

//função para controle de RESET no pino RTS do barramento rs485
void 
MBReceive_On_RS485( void )
{
    //essa checagem ocorre para saber se a recepção de dados na serial ja ocorreu, para que não seja atrapalhado a recepção de dados no buffer   
    int lenght_tam = 0;
    uart_get_buffered_data_len(EX_UART_NUM , (size_t*) &lenght_tam);
    while(lenght_tam != 0) {
        ESP_LOGI(TAG, "Buffer cheio on MBReceive_On_RS485!");
        uart_flush(EX_UART_NUM);
        uart_get_buffered_data_len(EX_UART_NUM , (size_t*) &lenght_tam);
    }

    vTaskDelay(5 / portTICK_PERIOD_MS);
    uart_set_rts(EX_UART_NUM, FALSE);
}