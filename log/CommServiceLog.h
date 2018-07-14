 
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
    static void LogSqlEvent(LPCTSTR pFormat, ...);
};

#endif
