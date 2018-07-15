/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/15 09:55:31

    Reversion:
        
-------------------------------------------------------------------------*/
 
#ifndef		__THREADAPP_H_
#define		__THREADAPP_H_

BOOL	gRun = FALSE;
LONG	glConnectNum = 0;					//	连接的客户数
DWORD 	gMaxWaitTime = 3000;


//	守护线程：等待调度中心客户端的连接	
DWORD	WINAPI	server_recv_thread(LPVOID p);
DWORD	WINAPI	AcceptWorkItem(PVOID pvContext);

#endif

