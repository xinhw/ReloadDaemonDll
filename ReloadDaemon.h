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

/*
报文处理回调函数
*/
typedef int (CALLBACK *processCallback)(PPACKAGEHEADER,WORD,BYTE *,WORD &,BYTE *); 
typedef void (CALLBACK *printMessageCallback)(char *);

HANDLE hdlThread = INVALID_HANDLE_VALUE;

processCallback theProc = NULL;
printMessageCallback thePrintMsg = NULL;

extern "C"
{
	int __stdcall beginTCPService(int nportno,int ntimeout);
	int __stdcall stopTCPService();
	void __stdcall setCallBack(processCallback proc);

	void __stdcall setPrintCallBack(printMessageCallback proc);
}

void logEvent(LPCTSTR pFormat, ...);
