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


#ifndef __MB_CONFIG_H__
#define __MB_CONFIG_H__

/* ----------------------- Modbus includes ----------------------------------*/
#include "defs.h"


/* ----------------------- Defines ------------------------------------------*/
#define MB_SLAVE

extern volatile UCHAR MB_SLAVE_ADDRESS; 

#define __ESP32__
#define DEBUG_MB TRUE

/* ----------------------- ENABLE FUNCTIONS-----------------------------------*/   

#if defined(__ESP32__)
    #define UART1
    //#define UART2 
    

#elif defined(__STM32__) 
    //#define UART0
    //#define UART1 
    #define UART2
#endif

 /* ----------------------- PORT --------------------------------------------*/
//#define __delay1ms()                _delay_ms(1) 
#define TIMEOUT_1MS                      1
//#define ENTER_CRITICAL_SECTION_MB()   cli() 
//#define EXIT_CRITICAL_SECTION_MB()    sei() 

/* ----------------------- Variables ----------------------------------------*/

#define MB_BUFFER_SIZE 						                      (255)

/* ----------------------- ENABLE FUNCTIONS-----------------------------------*/

#define ENABLE_MB_FUNCTION01_READ_COILS               ( 1 )
#define ENABLE_MB_FUNCTION03_READ_HOLDING_REGISTERS   ( 1 )
#define ENABLE_MB_FUNCTION06_WRITE_SINGLE_REGISTER    ( 1 )
#define ENABLE_MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS ( 1 )
#define ENABLE_MB_EXCEPTIONS                          ( 1 )

/* ----------------------- Defines ------------------------------------------*/
#define MB_FUNCTION01_READ_COILS                      ( 0X01 )
#define MB_FUNCTION03_READ_HOLDING_REGISTERS          ( 0X03 )
#define MB_FUNCTION06_WRITE_SINGLE_REGISTER           ( 0X06 )
#define MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS        ( 0X10 )

/* ----------------------- Exceptions----------------------------------------*/
#define MB_EXCEPTION_ILEGAL_DATA_VALUE                 (0X03)
#define MB_EXCEPTION_ILEGAL_FUNCTION                   (0X01)

/* ----------------------- Interfaces ---------------------------------------*/

#define MBEventReadCoils(x,y,z) 				MBSlaveFunc01CallBack(x,y,z) 
#define MBEventReadRegisters(x,y,z) 			MBSlaveFunc03CallBack(x,y,z) 
#define MBEventWriteSingleRegister(x,y) 		MBSlaveFunc06CallBack(x,y)
#define MBEventWriteMultipleRegisters(x,y,z)  	MBSlaveFunc16CallBack(x,y,z) 

/* ----------------------- Supporting functions -----------------------------*/

void MBInit( void );
BOOL MB( void );
void MBException( USHORT usExceptionCode );
BOOL MBSendToMasterException( void );

/* ----------------------- Supporting functions -----------------------------*/
BOOL MBSlaveFunc01CallBack( USHORT usStartAddress, UCHAR * ucCoils, USHORT  usNumberOfCoils );
BOOL MBSlaveFunc01ReadCoils( UCHAR ucSlaveAddress, UCHAR usNumberOfBytes,  UCHAR * ucWriteCoils, USHORT usNumberOfCoils);

BOOL MBSlaveFunc03CallBack( USHORT usStartAddress, UCHAR * ucCoils,  USHORT  usNumberOfRegisters );
BOOL MBSlaveFunc03ReadRegisters( UCHAR ucSlaveAddress,UCHAR * ucWrite, USHORT usNumberOfRegisters);

BOOL MBSlaveFunc06CallBack( USHORT usStartAddress, USHORT  usValue );
BOOL MBSlaveFunc06WriteSingleRegister( UCHAR ucSlaveAddress,USHORT usStartAddress,USHORT usWriteRegister);

BOOL MBSlaveFunc16CallBack( USHORT usStartAddress, UCHAR * wRegHoldingbuffer, USHORT  usNumberOfRegisters );
BOOL MBSlaveFunc16WriteMultipleRegisters( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usWriteRegister); 
/* ----------------------------------------------------------------------------*/

#endif