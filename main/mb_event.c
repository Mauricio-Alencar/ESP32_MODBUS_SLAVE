/* ----------------------- Modbus includes ----------------------------------*/

#include "defs.h"
#include "mb_config.h"
#include "mb_timer.h"
#include "mb_serial.h"
#include "mb_event.h"

/* ----------------------- Variables ----------------------------------------*/
static volatile BOOL MBEventStatus = FALSE;


BOOL
MBEventPost( BOOL Event )
{   //ENTER_CRITICAL_SECTION_MB( );
    MBUartRXDisable();

    MBEventStatus = Event;

    MBUartRXEnable();
    //EXIT_CRITICAL_SECTION_MB( );
    return TRUE;
}

BOOL
MBEventPostFromIRQ( BOOL Event )
{
    MBEventStatus = Event;
    return TRUE;
}

BOOL
MBEventGet( void )
{
    BOOL MBEvent;
    // ENTER_CRITICAL_SECTION_MB( );
    MBUartRXDisable();

    MBEvent =  MBEventStatus;

    MBUartRXEnable();
    // EXIT_CRITICAL_SECTION_MB( );
      return MBEvent;
}