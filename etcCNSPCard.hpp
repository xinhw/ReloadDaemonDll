#pragma once

#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#ifndef DWORD
#define DWORD unsigned long
#endif


#include "EncodeDLL.h"
#include "datatypedef.h"

#include ".\\log\\CommServiceLog.h"
#include ".\\net\\tcptransfer.h"
#include ".\\protocol\\Protol.h"

#include "ErrorCode.h"
#include ".\\misc\\misc.h"
#include ".\\app\\command.h"

#ifdef __DEBUG__
#define PRINTK printf
#else
#define PRINTK	CCommServiceLog::LogEvent
#pragma message("写日志功能开启")
#endif


#include ".\\log\\CommServiceLog.cpp"
#include ".\\net\\tcptransfer.cpp"
#include ".\\protocol\\Protol.cpp"
#include ".\\misc\\misc.cpp"
#include ".\\app\\command.cpp"


