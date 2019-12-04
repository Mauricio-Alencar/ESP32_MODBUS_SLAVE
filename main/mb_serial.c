#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

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
    
/* ----------------------- Variables ----------------------------------------*/
static const int RX_BUF_SIZE = 1024;
UCHAR mb_buffer[MB_BUFFER_SIZE];
UCHAR mb_buffer_indice = 0;


//vetores de interrupção da UART, em caso de uso para RTOS, tarefa com alimentação na recepção recepção serial.
static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t data[255];
    while (1) {
     
    uart_read_bytes(EX_UART_NUM, data, 1, portMAX_DELAY);

    mb_buffer[mb_buffer_indice] = data;
    mb_buffer_indice++;
    
    if(mb_buffer_indice >= MB_BUFFER_SIZE)
            mb_buffer_indice = 0;

    ESP_LOGI(RX_TASK_TAG, "Read: '%02X'", mb_buffer[mb_buffer_indice]);
    MBTimerRestart();

     /*
     data[rxBytes] = 0;
     ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
     ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
     */

    }

    vTaskDelete(NULL);
}

/*
void 
MBUartIRQ( void )
{  
    //Verifica se o BUFFER esta cheio, caso esteja, ele zera o indice para recepcionar novos dados.
    if(mb_buffer_indice >= MB_BUFFER_SIZE)
			mb_buffer_indice = 0;
    
//Executa a recepção dos dados na UART alimentando um buffer de acordo com indice.
#if defined UART1
    while(!(UCSR0A & (1<<RXC0)));	  
	mb_buffer[mb_buffer_indice++] = UDR0;
    
#elif defined UART2
    while(!(UCSR1A & (1<<RXC1)));	  
	mb_buffer[mb_buffer_indice++] = UDR1;
#endif     
}
*/

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
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

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
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

#endif    

    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

    return TRUE;
} 

//função para habilitação da recepção serial
void 
MBUartRXEnable( void )
{   
    MBUartInit();
	MBReceive_On_RS485(); 
}

//função para desabilitar a recepção serial
void 
MBUartRXDisable( void )
{
    uart_driver_delete(EX_UART_NUM);   
	MBTransmit_On_RS485();
}

//função para enviar um caracterer no barramento rs485
void
MBUartRxSend( UCHAR ch )
{
#if defined UART1
    //while(!(UCSR0A & (1<<UDRE0)));
    //UDR0 = ch;
#elif defined UART2 
   // while(!(UCSR1A & (1<<UDRE1))); 
   // UDR1 = ch;
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