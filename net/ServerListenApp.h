 
#ifndef		__THREADAPP_H_
#define		__THREADAPP_H_



BOOL	gRun = FALSE;
LONG	glConnectNum = 0;					//	连接的客户数
DWORD 	gMaxWaitTime = 3000;
DWORD	gMaxEncoderIdleTime = 0;


//	守护线程：等待调度中心客户端的连接	
DWORD	WINAPI	server_recv_thread(LPVOID p);
DWORD	WINAPI	AcceptWorkItem(PVOID pvContext);
//void	AcceptWorkItem(PVOID pvContext);
#endif

