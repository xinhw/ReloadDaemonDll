/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/08/01 19:52:04

    Reversion:
        
-------------------------------------------------------------------------*/
 

UINT	CCommServiceLog::WRITE_LOG = 0;
char	CCommServiceLog::COMM_SERVICE_LOG[0xff]={0};
HANDLE	CCommServiceLog::hLogFile = INVALID_HANDLE_VALUE;	

/*-------------------------------------------------------------------------
Function:		CCommServiceLog.CCommServiceLog
Created:		2018-08-01 19:53:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCommServiceLog::CCommServiceLog()
{
	hLogFile = INVALID_HANDLE_VALUE;	
}

/*-------------------------------------------------------------------------
Function:		CCommServiceLog.~CCommServiceLog
Created:		2018-08-01 19:53:47
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCommServiceLog::~CCommServiceLog()
{
	if(hLogFile !=INVALID_HANDLE_VALUE) CloseHandle(hLogFile);
	hLogFile = INVALID_HANDLE_VALUE;
}

/*-------------------------------------------------------------------------
Function:		CCommServiceLog.init
Created:		2018-08-01 19:53:50
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void CCommServiceLog::init()
{
	char		strCurDir[MAX_PATH];
	SYSTEMTIME	st;

	GetLocalTime(&st);
	//	当前目录
	memset(strCurDir,0x00,MAX_PATH);
	GetCurrentDirectory(MAX_PATH,strCurDir);

	//	日志文件
	memset(COMM_SERVICE_LOG,0x00,0xff);
	sprintf(COMM_SERVICE_LOG,"%s\\log\\ICService%04d%02d%02d.log",strCurDir,st.wYear,st.wMonth,st.wDay);
	return;
}

/*-------------------------------------------------------------------------
Function:		CCommServiceLog.LogEvent
Created:		2018-08-01 19:53:53
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void CCommServiceLog::LogEvent(LPCTSTR pFormat, ...)
{
	TCHAR		chMsg[1024];
	va_list		pArg;

	DWORD		nFileSize;
	DWORD		wWritten;
	/*
	SYSTEMTIME	st;
	char		pszMsg[1024];	
	*/
	va_start(pArg, pFormat);
	vsprintf(chMsg, pFormat, pArg);
	va_end(pArg);
	
	if(hLogFile ==INVALID_HANDLE_VALUE)
	{
		init();
		hLogFile = CreateFile(COMM_SERVICE_LOG,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		GetFileSize(hLogFile,&nFileSize);
		SetFilePointer(hLogFile,nFileSize,NULL,FILE_END);
	}
	
	if(hLogFile)
	{
		/*
		GetLocalTime(&st);
		
		sprintf(pszMsg,"%04d-%02d-%02d %02d:%02d:%02d\t\t%s\r\n",
			st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,
			chMsg);
		*/
		WriteFile(hLogFile,chMsg,strlen(chMsg),&wWritten,NULL);
	}
	
	return;
}
