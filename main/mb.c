#include "mb_config.h"
#include "mb_crc.h"
#include "mb_timer.h"
#include "mb_serial.h"
#include "mb_event.h"
#include "defs.h"
#include "ESP32_ModbusRTU_Slave.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

/* ----------------------- Variables----------------------------------------*/

struct SMBException
{
  UCHAR MBFunctionCode;
  UCHAR MBExceptionStatus;
  UCHAR MBExceptionCode;
} sMBException;

static const char *TAG = "MB_PROCESS";

#ifdef MB_SLAVE 

static BOOL 
MBSlaveEvent( void  )
{
    UCHAR ucWrite[MB_BUFFER_SIZE];
    BOOL        Retorno = TRUE;
    USHORT      usCRC; 
    UCHAR       ucHigh, ucLow;

    //desabilita timer e serial para tratar o evento de recepção, garantindo que nada irá modificar esse buffer
    MBTimerOff();
	MBUartRXDisable();

    ESP_LOGI(TAG, "evento de envio gerado!");

    //verifica se o indice do buffer tem conteudo
    if(mb_buffer_indice > 0)
    {
        //verifica se o indice 0 do buffer é o endereço do slave.
        if( mb_buffer[0] == MB_SLAVE_ADDRESS )
        {

            /* MB_FUNCTION01_READ_COILS */  
#if ENABLE_MB_FUNCTION01_READ_COILS > 0

            if( mb_buffer[1] == MB_FUNCTION01_READ_COILS )
            {
                usCRC = usMBCRC16( mb_buffer, 6 );         
                ucHigh  = (UCHAR)(usCRC>>8);
                ucLow   =  (UCHAR)(usCRC);
             
                if( mb_buffer[7] == ucHigh && mb_buffer[6] == ucLow)
                {             
                    UCHAR usNumberOfBytes;
                    //concatena o indice 2 e 3 para pegar o endereço de inicio do protoclo modbus
                    USHORT usStartAddress = (mb_buffer[2] << 8)| mb_buffer[3];
                    //concatena o indice 4 e 5 para pegar o endereço o numero de coils a serem lidos
                    USHORT usNumberOfCoils = (mb_buffer[4] << 8)| mb_buffer[5];
                    
                    //verifica se toda a informação cabe em apenas um unico BYTE ou se precisa adicionar mais um byte a informação
                    if( ( usNumberOfCoils & 0x0007 ) != 0 )
                    {
                        usNumberOfBytes = ( UCHAR )( usNumberOfCoils / 8 + 1 );
                    }
                    else
                    {
                        usNumberOfBytes = ( UCHAR )( usNumberOfCoils / 8 );
                    }

                    //chamada de função de callback no codigo main
                    MBSlaveFunc01CallBack( usStartAddress, ucWrite,  usNumberOfCoils );
                    //chamada de função para retornar os valores pro MASTER
                    MBSlaveFunc01ReadCoils( MB_SLAVE_ADDRESS, usNumberOfBytes, ucWrite, usNumberOfCoils);
                } else {
                    Retorno = FALSE;
                    sMBException.MBFunctionCode =  MB_FUNCTION01_READ_COILS;
                    sMBException.MBExceptionStatus = TRUE;
                    sMBException.MBExceptionCode = MB_EXCEPTION_ILEGAL_DATA_VALUE;
                }
            }  
#endif
            /* MB_FUNCTION03_READ_HOLDING_REGISTERS */
#if ENABLE_MB_FUNCTION03_READ_HOLDING_REGISTERS > 0

            if( mb_buffer[1] == MB_FUNCTION03_READ_HOLDING_REGISTERS )
            {
                usCRC = usMBCRC16(mb_buffer, 6 );         
                ucHigh = (UCHAR)(usCRC>>8);
                ucLow =  (UCHAR)(usCRC);
                if( mb_buffer[7] == ucHigh && mb_buffer[6] == ucLow )
                {
                    USHORT usStartAddress = (mb_buffer[2] << 8) | mb_buffer[3];
                    USHORT usNumberOfPoints = (mb_buffer[4] << 8) | mb_buffer[5];
                    
                    MBSlaveFunc03CallBack( usStartAddress, ucWrite,  usNumberOfPoints );
                    MBSlaveFunc03ReadRegisters( MB_SLAVE_ADDRESS, ucWrite, usNumberOfPoints );
                
                } else {
                    Retorno = FALSE;
                    sMBException.MBFunctionCode =  MB_FUNCTION03_READ_HOLDING_REGISTERS;
                    sMBException.MBExceptionStatus = TRUE;
                    sMBException.MBExceptionCode = MB_EXCEPTION_ILEGAL_DATA_VALUE;
                }

            } 
#endif
      /* MB_FUNCTION06_WRITE_SINGLE_REGISTER */
#if ENABLE_MB_FUNCTION06_WRITE_SINGLE_REGISTER > 0

            if( mb_buffer[1] == MB_FUNCTION06_WRITE_SINGLE_REGISTER )
            {
                usCRC = usMBCRC16(mb_buffer, 6 );         
                ucHigh = (UCHAR)(usCRC>>8);
                ucLow =  (UCHAR)(usCRC);

                if( mb_buffer[7] == ucHigh && mb_buffer[6] == ucLow)
                {
                    USHORT usStartAddress = (mb_buffer[2] << 8) | mb_buffer[3];
                    USHORT usValue = (mb_buffer[4] << 8) | mb_buffer[5];

                    MBSlaveFunc06CallBack( usStartAddress, usValue );
                    MBSlaveFunc06WriteSingleRegister( MB_SLAVE_ADDRESS, usStartAddress, usValue );

                } else {
                    Retorno = FALSE;
                    sMBException.MBFunctionCode =  MB_FUNCTION06_WRITE_SINGLE_REGISTER;
                    sMBException.MBExceptionStatus = TRUE;
                    sMBException.MBExceptionCode = MB_EXCEPTION_ILEGAL_DATA_VALUE;
                }

            } 
#endif 
             /* MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS */
#if ENABLE_MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS > 0

            if( mb_buffer[1] == MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS )
            {
                usCRC = usMBCRC16(mb_buffer, 6 + mb_buffer[6] + 1 );         
                ucHigh = (UCHAR)(usCRC>>8);
                ucLow =  (UCHAR)(usCRC);

                if( mb_buffer[6 + mb_buffer[6] + 2] == ucHigh && mb_buffer[6 + mb_buffer[6] + 1] == ucLow)
                {
                    USHORT usStartAddress = (mb_buffer[2] << 8)| mb_buffer[3];
                    USHORT usWriteRegister = (mb_buffer[4] << 8)| mb_buffer[5];
                    USHORT usNumberOfPoints =  mb_buffer[6];
 
                    MBSlaveFunc16CallBack( usStartAddress, &mb_buffer[7],  usWriteRegister  );         
                    MBSlaveFunc16WriteMultipleRegisters( MB_SLAVE_ADDRESS, usStartAddress, usWriteRegister );

                } else {
                    Retorno = FALSE;
                    sMBException.MBFunctionCode = MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS;
                    sMBException.MBExceptionStatus = TRUE;
                    sMBException.MBExceptionCode = MB_EXCEPTION_ILEGAL_DATA_VALUE;
                }

            }
#endif
#if ENABLE_MB_EXCEPTIONS > 0

                if(sMBException.MBExceptionStatus)
                {
                    //Se Precisar tratar a Excessão pela função de callback no main
                    MBException(sMBException.MBFunctionCode);
                    
                    MBSendToMasterException( );
                    sMBException.MBExceptionStatus = FALSE;
                }
#endif
        }
    }

    mb_buffer_indice = 0;
	MBUartRXEnable();
	
    return Retorno;
}

/* ######################################################################### */ 
/* MB Slave: EXCEPTION -SLAVE SEND TO MASTER */

BOOL
MBSendToMasterException(  )
{
        USHORT usCRC;
        UCHAR i;
        UCHAR frame[5];

        MBUartRXDisable();

        frame[0] = MB_SLAVE_ADDRESS;
        frame[1] = 0x80 + sMBException.MBFunctionCode;
        frame[2] = sMBException.MBExceptionCode;
        usCRC = usMBCRC16(frame, 3);
        frame[3] = (UCHAR)(usCRC & 0xFF);
        frame[4] = (UCHAR)((usCRC >> 8) & 0xFF);

        for(i = 0; i < sizeof(frame); ++i)
        {
              MBUartRxSend( ( const char ) frame[i] );
        }

        MBUartRXEnable();

        return TRUE;
}

/* ######################################################################### */ 
/* MB Slave: MB_FUNCTION01_READ_COILS */

BOOL
MBSlaveFunc01ReadCoils(
                            UCHAR ucSlaveAddress, 
                            UCHAR usNumberOfBytes,   
                            UCHAR * ucWriteCoils, 
                            USHORT usNumberOfCoils) 
{
        USHORT usCRC;
        UCHAR i = 0, t = 0, y = 0;
        UCHAR frame[MB_BUFFER_SIZE];

        MBUartRXDisable(); 
         
        if(usNumberOfCoils == 0) return FALSE;

        frame[0] = ucSlaveAddress;
        frame[1] = MB_FUNCTION01_READ_COILS;
        frame[2] = usNumberOfBytes;

        while(usNumberOfBytes--)
        {
              frame[3 + i++] = (UCHAR)(ucWriteCoils[t++]);
        }
        
        usCRC = usMBCRC16(frame, 3 + i);         
        frame[3 + i++] = (UCHAR)(usCRC & 0xFF);
        frame[3 + i++] = (UCHAR)((usCRC >> 8) & 0xFF);

        for(y = 0; y < 3 + i ; ++y)
        {
            MBUartRxSend( (const char) frame[y] );
        }

        MBUartRXEnable();

        return TRUE;
}

/* ######################################################################### */ 
/* MB Slave: MB_FUNCTION03_READ_HOLDING_REGISTERS */

BOOL
MBSlaveFunc03ReadRegisters(
                            UCHAR ucSlaveAddress, 
                            UCHAR * ucWrite, 
                            USHORT usNumberOfPoints) 
{
        USHORT usCRC;
        UCHAR i = 0, t = 0, y = 0;
        UCHAR frame[MB_BUFFER_SIZE];

        MBUartRXDisable(); 

        if(usNumberOfPoints == 0) return -1;

        frame[0] = ucSlaveAddress;
        frame[1] = MB_FUNCTION03_READ_HOLDING_REGISTERS;                 
        frame[2] = (UCHAR)(usNumberOfPoints*2);

        while(usNumberOfPoints--)
        {
              frame[3 + i++] = (UCHAR)(ucWrite[t++ +1]);
              frame[3 + i++] = (UCHAR)(ucWrite[t++ -1]);
        }
        
        usCRC = usMBCRC16(frame, 3 + i);         
        frame[3 + i++] = (UCHAR)(usCRC & 0xFF);
        frame[3 + i++] = (UCHAR)((usCRC >> 8) & 0xFF);

        for(y = 0; y < 3 + i ; ++y)
        {
            MBUartRxSend( (const char) frame[y] );
        }

        MBUartRXEnable();

        return TRUE;
}

//06 Function write Holding Register
BOOL
MBSlaveFunc06WriteSingleRegister(
                            UCHAR ucSlaveAddress, 
                            USHORT usStartAddress, 
                            USHORT usWriteRegister) 
{
        USHORT usCRC;
        UCHAR i;
        UCHAR frame[8];
        
        MBUartRXDisable(); 
         
        frame[0] = ucSlaveAddress;               
        frame[1] = MB_FUNCTION06_WRITE_SINGLE_REGISTER;                   
        frame[2] = (UCHAR)(usStartAddress >> 8);  
        frame[3] = (UCHAR)usStartAddress;         
        frame[4] = (UCHAR)(usWriteRegister >> 8);
        frame[5] = (UCHAR)usWriteRegister;      
        usCRC = usMBCRC16(frame, 6);         
        frame[6] = (UCHAR)(usCRC & 0xFF);   
        frame[7] = (UCHAR)((usCRC >> 8) & 0xFF); 

        for(i = 0; i < sizeof(frame); ++i)
        {
              MBUartRxSend( (const char) frame[i] );
        }
        
        MBUartRXEnable();

        return TRUE;
}

//16 Function write Multiple Registers
BOOL
MBSlaveFunc16WriteMultipleRegisters(
                            UCHAR ucSlaveAddress, 
                            USHORT usStartAddress, 
                            USHORT usWriteRegister) 
{
        USHORT usCRC;
        UCHAR i;
        UCHAR frame[8];
        
        MBUartRXDisable(); 
         
        frame[0] = ucSlaveAddress;               
        frame[1] = MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS;                   
        frame[2] = (UCHAR)(usStartAddress >> 8);  
        frame[3] = (UCHAR)usStartAddress;         
        frame[4] = (UCHAR)(usWriteRegister >> 8);
        frame[5] = (UCHAR)usWriteRegister;      
        usCRC = usMBCRC16(frame, 6);         
        frame[6] = (UCHAR)(usCRC & 0xFF);   
        frame[7] = (UCHAR)((usCRC >> 8) & 0xFF); 

        for(i = 0; i < sizeof(frame); ++i)
        {
              MBUartRxSend( (const char) frame[i] );
        }
        
        MBUartRXEnable();

        return TRUE;
}


#endif //MB_SLAVE
/* ######################################################################## */
/* MBLoop:
 * Função responsável em analisar se um frame foi completamente recebido.
 * Caso sim, chama a função para o processamento do frame.
 */

BOOL MB( void )
{
     BOOL EventStatus = FALSE;
     //MBEventGET verifica se chegou algo valido para ser lido no buffer do modbus
     if( MBEventGet() )
     {  
        //MBSlaveEvent faz a decodificação dos pacotes recebidos 
        MBSlaveEvent();

        //é zerado a flag de leitura do MBEvent
        MBEventPost(FALSE);
        EventStatus = TRUE;
     }
     return EventStatus;
}

//função responsavel por inicializar a serial, interrupções globais e timer
void MBInit( void )
{
    MBTimerInit();
    MBUartInit();
    //MBUartIE();
}
