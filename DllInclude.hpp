#ifndef __DLLINCLUDE_H__
#define __DLLINCLUDE_H__

#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "EncodeDLL.h"
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
#include ".\\reader\\JTReader.h"
#include ".\\reader\\JYRsuReader.h"
#include ".\\card\\cardbase.h"
#include ".\\card\\tycpucard.h"
#include ".\\card\\obucard.h"
#include ".\\card\\wd3descard.h"
#include ".\\card\\jd3descard.h"

#ifdef __DEBUG__
#define PRINTK printf
#else
#define PRINTK	CCommServiceLog::LogEvent
#pragma message("д��־���ܿ���")
#endif


#include ".\\log\\CommServiceLog.cpp"
#include ".\\net\\tcptransfer.cpp"
#include ".\\protocol\\Protol.cpp"
#include ".\\misc\\misc.cpp"
#include ".\\app\\command.cpp"
#include ".\\reader\\NXRsuReader.cpp"
#include ".\\reader\\AISinoReader.cpp"
#include ".\\reader\\JTReader.cpp"
#include ".\\reader\\JYRsuReader.cpp"
#include ".\\card\\cardbase.cpp"
#include ".\\card\\tycpucard.cpp"
#include ".\\card\\obucard.cpp"
#include ".\\card\\wd3descard.cpp"
#include ".\\card\\jd3descard.cpp"

#endif
