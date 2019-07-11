/*-------------------------------------------------------------------------
  Shanghai Huahong Jitong Smart Card System Co., Ltd  
  Dept:		R & D  
  Author:		Hsin Honway(xinhongwei@sina.com)  
  Time:		2005-7-20 21:51:19
  Description:	
  
  Revision History:
  
-------------------------------------------------------------------------*/
#include <WinSock2.h>

#ifndef	__TRANSFER_H_
#define	__TRANSFER_H_


class CTcpTransfer
{
public:
	CTcpTransfer();
	~CTcpTransfer();
	
private:
	SOCKET	sLocal;					//	本地的Socket
	void	ShowWSAErrorMsg(int nErrNo);

public:
	int		init_socket();
	//		初始化连接
	int		connect_server(UINT	nPort,char	*pszIPAddr);	

	//		终止连接
	int		disconnect();

	int		s_send(WORD	rBytes,UCHAR	*pszBuf);

	//		2016-10-11
	int		s_recv(WORD *rBytes, UCHAR	*pszBuf, WORD wMaxBytes, UINT	nWaitMaxTime = 3000);

public:
	BOOL bConnected;

};

#endif





















