/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/08/01 19:55:42

    Reversion:
        
-------------------------------------------------------------------------*/
 
#ifndef		__COMMSERVICELOG_H_
#define		__COMMSERVICELOG_H_

class CCommServiceLog
{
public:
	CCommServiceLog();
	~CCommServiceLog();
	
private:	
	//	记录日志的宏
	static UINT		WRITE_LOG;
	static char	COMM_SERVICE_LOG[0xff];
	static HANDLE	hLogFile;	
	
	static void init();
public:	
	static void LogEvent(LPCTSTR pFormat, ...);
};

#endif
