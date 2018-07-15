/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/15 09:43:02

    Reversion:
        
-------------------------------------------------------------------------*/
// ReloadDaemon.cpp : Defines the entry point for the DLL application.
//

#include "include.h"



BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}





/*-------------------------------------------------------------------------
Function:		beginTCPService
Created:		2018-07-15 09:43:06
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall beginTCPService(int nportno,int ntimeout)
{
	int			ret;
	WSADATA		wsaData;  	
	PTHREADPARAM ptp;
	
	//	WinSock2初始化
	WORD wSockVersion = MAKEWORD(2,1);
	ret = WSAStartup(wSockVersion,&wsaData);
	if(ret) 
	{
		PRINTK("\nWIN SOCKET2初始化失败！");
		return -1;
	}	

	ptp =(PTHREADPARAM)malloc(sizeof(THREADPARAM));
	ptp->nportno = nportno;
	ptp->ntimeout = ntimeout;

	gRun = TRUE;

	//	接收充值机连接线程(TCP)
	hdlThread = CreateThread(NULL,NULL,server_recv_thread,(PVOID)ptp,0,NULL);
	if(hdlThread==INVALID_HANDLE_VALUE) return -2;
	
	SetThreadPriority(hdlThread,THREAD_PRIORITY_HIGHEST);
	return 0;
}




/*-------------------------------------------------------------------------
Function:		stopTCPService
Created:		2018-07-15 09:43:11
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall stopTCPService()
{
	gRun = FALSE;

	Sleep(100);

	WaitForSingleObject(hdlThread,INFINITE);

	PRINTK("\n服务停止！");
	WSACleanup();
	return 0;
}




/*-------------------------------------------------------------------------
Function:		setCallBack
Created:		2018-07-15 09:43:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall setCallBack(processCallback proc)
{
	theProc = proc;
	return;
}



/*-------------------------------------------------------------------------
Function:		setPrintCallBack
Created:		2018-07-15 09:43:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall setPrintCallBack(printMessageCallback proc)
{
	thePrintMsg = proc;
}


/*-------------------------------------------------------------------------
Function:		logEvent
Created:		2018-07-15 09:43:29
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void logEvent(LPCTSTR pFormat, ...)
{

	TCHAR		chMsg[1024];
	va_list		pArg;
	
	va_start(pArg, pFormat);
	vsprintf(chMsg, pFormat, pArg);
	va_end(pArg);
	
	if(NULL!=thePrintMsg) thePrintMsg(0,chMsg);
	return;
}

void logEvent(int nType,LPCTSTR pFormat, ...)
{
	TCHAR		chMsg[1024];
	va_list		pArg;
	
	va_start(pArg, pFormat);
	vsprintf(chMsg, pFormat, pArg);
	va_end(pArg);
	
	if(NULL!=thePrintMsg) thePrintMsg(nType,chMsg);
	return;
}