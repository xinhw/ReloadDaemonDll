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

int __stdcall stopTCPService()
{
	gRun = FALSE;
	Sleep(1);

	WaitForSingleObject(hdlThread,INFINITE);

	WSACleanup();
	return 0;
}

void __stdcall setCallBack(processCallback proc)
{
	theProc = proc;
	return;
}

void __stdcall setPrintCallBack(printMessageCallback proc)
{
	thePrintMsg = proc;
}


void logEvent(LPCTSTR pFormat, ...)
{
	TCHAR		chMsg[1024];
	va_list		pArg;
	
	va_start(pArg, pFormat);
	vsprintf(chMsg, pFormat, pArg);
	va_end(pArg);
	
	if(NULL!=thePrintMsg) thePrintMsg(chMsg);

	
	return;
}