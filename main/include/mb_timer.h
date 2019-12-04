
#ifndef __MB_TIMER_H__
#define __MB_TIMER_H__

/* ----------------------- Modbus includes ----------------------------------*/

#include "defs.h"

/* ----------------------- Supporting functions -----------------------------*/

BOOL 		MBTimerIRQ( void );
BOOL		MBTimerInit( void );
BOOL		MBTimerOff( void );
BOOL		MBTimerOn( void );
BOOL 		MBTimerRestart( void );

#endif