 
#ifndef		__INCLUDE_H_
#define		__INCLUDE_H_
	#include <afxdb.h>
	#include <windows.h>
	#include <string.h>
	#include <stdio.h>
	#include <time.h>
	#include <setjmp.h>
	#include <process.h>
	#include <winsock2.h>
	#include <Nb30.h> 
	#include <time.h>
	#include <afxinet.h>
	#include <conio.h>
	#include "datatypedef.h"

	#include ".\\log\\CommServiceLog.h"
	#include ".\\net\\transfer.h"		
	#include ".\\net\\NetTool.h"
	#include ".\\net\\ServerListenApp.h"
	#include ".\\protocol\\Protol.h"

	#include "ErrorCode.h"
	#include ".\\misc\\ThreadDispatch.h"
	#include ".\\misc\\misc.h"
	#include "ReloadDaemon.h"


	#ifdef __DEBUG__
	#define PRINTK printf
	#else
	#define PRINTK	logEvent
	#endif


	#include ".\\log\\CommServiceLog.cpp"
	#include ".\\net\\NetTool.cpp"
	#include ".\\net\\ServerListenApp.cpp"

	#include ".\\protocol\\Protol.cpp"

	//2014-03-12
	#include ".\\misc\\ThreadDispatch.cpp"
	#include ".\\misc\\misc.cpp"
#endif
