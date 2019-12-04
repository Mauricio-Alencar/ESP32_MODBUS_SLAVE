
/* ----------------------- Modbus includes ----------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "mb_timer.h"
#include "mb_event.h"

static void periodic_timer_callback(void* arg);

/* ----------------------- Variables-----------------------------------------*/
esp_timer_handle_t periodic_timer;

static USHORT   usTimerOCRADelta;
static const char *TAG = "MB_SERIAL";

/* ----------------------- Start implementation -----------------------------*/

static void periodic_timer_callback(void* arg)
{
    MBTimerIRQ();
}

//função para detecção de timeout para finalização de pacote de 3.5char
BOOL 
MBTimerIRQ( void )
{
    //é desligado o timer para que não fique gerando varios timeouts
    MBTimerOff();
    ESP_LOGI(TAG, "TIMER OVF, EVENTO DE RECEPÇÃO SINALIZADO!");
    //função que executa o o SET de flag para controle de evento, esse metodo é utilizado para facilitar a portabilidade no uso de funções de modbus para FREERTOS como event group ou queue.
    MBEventPostFromIRQ(TRUE);
       
	return TRUE;
}

//inicializa timer
BOOL
MBTimerInit( void )
{ 
   const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            /* nome é opicional, mas é util para debug */
            .name = "timer_debug_modbus"
    };    

    /* O temporizador é criado como um handle mas não é inicializado  */

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_LOGI(TAG, "TIMER CRIADO!");
    MBTimerOff();

	return TRUE;
}

//função para desligar o timer do MB
BOOL
MBTimerOff( void )
{
    //para o timer
    ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
    ESP_LOGI(TAG, "TIMER OFF!");
    
	return TRUE;
}

//função para ligar o timer do MB
BOOL
MBTimerOn( void )
{
   /* Inicia o timer */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000));
    ESP_LOGI(TAG, "TIMER ON!");

	return TRUE;
}

//função para reinicilizar o timer;
BOOL 
MBTimerRestart( void )
{ 
    MBTimerOff();
    MBTimerOn();
	return TRUE;
}