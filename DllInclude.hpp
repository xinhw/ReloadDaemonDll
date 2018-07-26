#ifndef __DLLINCLUDE_H__
#define __DLLINCLUDE_H__

#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "datatypedef.h"

#include ".\\log\\CommServiceLog.h"
#include ".\\net\\tcptransfer.h"		
#include ".\\protocol\\Protol.h"

#include "ErrorCode.h"
#include ".\\misc\\misc.h"
#include ".\\app\\command.h"
#include ".\\reader\\CardReader.h"
#include "etcCNSPCard.h"

#include ".\\reader\\NXRsuReader.h"
#include ".\\reader\\AISinoReader.h"
#include ".\\card\\cardbase.h"
#include ".\\card\\tycpucard.h"
#include ".\\card\\obucard.h"

#ifdef __DEBUG__
#define PRINTK printf
#else
#define PRINTK	CCommServiceLog::LogEvent
#endif


#include ".\\log\\CommServiceLog.cpp"
#include ".\\net\\tcptransfer.cpp"
#include ".\\protocol\\Protol.cpp"
#include ".\\misc\\misc.cpp"
#include ".\\app\\command.cpp"
#include ".\\reader\\NXRsuReader.cpp"
#include ".\\reader\\AISinoReader.cpp"
#include ".\\card\\cardbase.cpp"
#include ".\\card\\tycpucard.cpp"
#include ".\\card\\obucard.cpp"

#endif
