#ifndef __MB_EVENT_H_
#define __MB_EVENT_H_

/* ----------------------- Modbus includes ----------------------------------*/

#include "defs.h"

/* ----------------------- Supporting functions -----------------------------*/

BOOL		MBEventPost( BOOL Event );
BOOL		MBEventGet( void );
BOOL		MBEventPostFromIRQ( BOOL Event );
#endif