#ifndef __UART_H__
#define __UART_H__

/* ----------------------- Modbus includes ----------------------------------*/

#include "defs.h"

/* ----------------------- Supporting functions -----------------------------*/

void 	MBUartIRQ( void );
BOOL 	MBUartInit( void );
void 	MBUartRXEnable( void );
void 	MBUartRXDisable( void );
void 	MBUartRxSend( UCHAR ch );
void 	MBUartIE( void );

void    MBReceive_On_RS485( void );
void    MBTransmit_On_RS485( void );

extern  UCHAR mb_buffer[MB_BUFFER_SIZE];
extern  UCHAR mb_buffer_indice;

#endif