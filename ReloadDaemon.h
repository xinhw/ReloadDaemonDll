#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RELOADDAEMON_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RELOADDAEMON_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef RELOADDAEMON_EXPORTS
#define RELOADDAEMON_API __declspec(dllexport)
#else
#define RELOADDAEMON_API __declspec(dllimport)
#endif

/*	报文处理回调函数*/
typedef int (CALLBACK *processCallback)(PPACKAGEHEADER,WORD,BYTE *,WORD &,BYTE *); 
processCallback theProc = NULL;

/*	信息显示回调函数*/
typedef void (CALLBACK *printMessageCallback)(int,char *);
printMessageCallback thePrintMsg = NULL;


/*	线程句柄*/
HANDLE hdlThread = INVALID_HANDLE_VALUE;


extern "C"
{
	/*	开始服务*/
	int __stdcall beginTCPService(int nportno,int ntimeout);

	/*	停止服务*/
	int __stdcall stopTCPService();

	/*	设置报文处理回调函数*/
	void __stdcall setCallBack(processCallback proc);

	/*	设置信息显示回调函数*/
	void __stdcall setPrintCallBack(printMessageCallback proc);
}

void logEvent(LPCTSTR pFormat, ...);
void logEvent(int nType,LPCTSTR pFormat, ...);