 
#ifndef		__THREADAPP_H_
#define		__THREADAPP_H_



BOOL	gRun = FALSE;
LONG	glConnectNum = 0;					//	���ӵĿͻ���
DWORD 	gMaxWaitTime = 3000;
DWORD	gMaxEncoderIdleTime = 0;


//	�ػ��̣߳��ȴ��������Ŀͻ��˵�����	
DWORD	WINAPI	server_recv_thread(LPVOID p);
DWORD	WINAPI	AcceptWorkItem(PVOID pvContext);
//void	AcceptWorkItem(PVOID pvContext);
#endif

