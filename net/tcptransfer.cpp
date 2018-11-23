/*-------------------------------------------------------------------------
  Shanghai Huahong Jitong Smart Card System Co., Ltd  
  Dept:		R & D  
  Author:		Hsin Honway(xinhongwei@sina.com)  
  Time:		2005-7-20 21:52:45
  Description:	
  
  Revision History:
			����һ��TCP���������ӡ�
-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------
Function:		CTcpTransfer.CTcpTransfer
Created:		2016-08-23 10:17:47
Author:		Xin Hongwei(xinhw@huahongjt.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTcpTransfer::CTcpTransfer()
{
	bConnected = FALSE;
	sLocal = INVALID_SOCKET;
}

/*-------------------------------------------------------------------------
Function:		CTcpTransfer.~CTcpTransfer
Created:		2016-08-23 10:17:45
Author:		Xin Hongwei(xinhw@huahongjt.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTcpTransfer::~CTcpTransfer()
{
	if(sLocal!=INVALID_SOCKET) closesocket(sLocal);
	sLocal = INVALID_SOCKET;
}


/*-------------------------------------------------------------------------
Function:		CTcpTransfer.init_socket
Created:		2016-08-23 10:17:42
Author:		Xin Hongwei(xinhw@huahongjt.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTcpTransfer::init_socket()
{
	BOOL	val = TRUE;
	int		ret = 0;

	if (sLocal != INVALID_SOCKET)
	{
		closesocket(sLocal);
		sLocal = INVALID_SOCKET;
	}

	//	����Socket
	sLocal = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sLocal == INVALID_SOCKET) return -1;

	if (setsockopt(sLocal, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val)) != 0)
	{
		ret = WSAGetLastError();
		closesocket(sLocal);
		sLocal = INVALID_SOCKET;
		return -2;
	}

	return 0;
}



/*-------------------------------------------------------------------------
	Function Name: 	HD_Init
	Time:			2005-7-21 9:26:04
	Author:			Hsin Honway(xinhongwei@sina.com)
	Parameters:
					nPort		[in]		Զ�̵Ķ˿ں�
					pszIPAddr	[in]		Զ�̵�IP��ַ
					nLocalPort	[in]		���صĶ˿ںţ���ѡ��
											ȱʡΪ8900
	Return:
	
	Remarks:
	
-------------------------------------------------------------------------*/
int	CTcpTransfer::connect_server(UINT nPort,char *pszIPAddr)
{
	int				ret;
	SOCKADDR_IN		sRemoteAddr;		//	��ֵ����Addr

	if(sLocal==INVALID_SOCKET) return -1;

	//	Ŀ�ĵ�IP��ַ�Ͷ˿ں�
	sRemoteAddr.sin_family = AF_INET;
	sRemoteAddr.sin_port = htons(nPort);
	sRemoteAddr.sin_addr.S_un.S_addr = inet_addr(pszIPAddr);	
	
	bConnected = FALSE;

	 //	Connect��������
	ret = connect(sLocal, (LPSOCKADDR)&sRemoteAddr, sizeof(struct sockaddr));
	if(ret) 
	{
		ret = WSAGetLastError();
		//ShowWSAErrorMsg(ret);
		return -5;
	}
	
	bConnected = TRUE;

	return 0;
}





/*-------------------------------------------------------------------------
	Function Name: 	HD_Term
	Time:			2005-7-21 9:27:18
	Author:			Hsin Honway(xinhongwei@sina.com)
	Parameters:
	
	Return:
	
	Remarks:
					��ֹsocket��ж��winsock32.
-------------------------------------------------------------------------*/
int	CTcpTransfer::disconnect()
{
	if(sLocal!=INVALID_SOCKET) closesocket(sLocal);
	sLocal = INVALID_SOCKET;
	bConnected = FALSE;

	return 0;
}



/*-------------------------------------------------------------------------
	Function Name: 	s_send
	Time:			2005-7-21 9:27:45
	Author:			Hsin Honway(xinhongwei@sina.com)
	Parameters:
					rBytes	[in]		�����͵��ֽ���
					pszBuf	[in]		�����͵��ֽ�
	Return:
	
	Remarks:
	
-------------------------------------------------------------------------*/
int		CTcpTransfer::s_send(WORD	rBytes,UCHAR	*pszBuf)
{
	int		ret;

	struct timeval waittm;
	fd_set	fd;
	
	if(sLocal ==INVALID_SOCKET) return -108;

#ifdef DEBUG_TCP_PRINT
	WORD i;
	for(i=0;i<rBytes;i++) PRINTK("%02X ",pszBuf[i]);
#endif	

	//	���ü������ʱ��
	waittm.tv_sec = 1;
	waittm.tv_usec = 0;
	
	FD_ZERO(&fd);
	FD_SET(sLocal,&fd);
	
	//	����Ƿ���״̬׼���õĿɶ���socket.
	ret = select(0,NULL,&fd,NULL,&waittm);
	if(ret<1) return -10;
	
	ret = send(sLocal,(char *)pszBuf,rBytes,0);
	if(ret<0) 
	{
		ret = WSAGetLastError();
		//ShowWSAErrorMsg(ret);
		return -11;
	}
	if((WORD)ret!=rBytes) return -12;
	
	return 0;
}




/*-------------------------------------------------------------------------
Function:		CTcpTransfer.s_recv
Created:		2016-10-13 17:26:17
Author:		Xin Hongwei(xinhw@huahongjt.com)
Parameters: 
        
Reversion:
			nWaitMaxTime	���ʱ�����룩        
-------------------------------------------------------------------------*/
int		CTcpTransfer::s_recv(WORD *rBytes, UCHAR	*pszBuf, WORD wMaxBytes, UINT	nWaitMaxTime)
{
	int		ret;

	struct timeval waittm;
	fd_set	fd;

	if (sLocal == INVALID_SOCKET) return -108;

	//	���ü������ʱ��
	waittm.tv_sec = nWaitMaxTime/1000;
	waittm.tv_usec = nWaitMaxTime%1000;

	FD_ZERO(&fd);
	FD_SET(sLocal, &fd);

	//	����Ƿ���״̬׼���õĿɶ���socket.
	ret = select(0, &fd, NULL, NULL, &waittm);
	if (ret<1) return -21;

	//	���ܿͻ��˵ķ��͡�
	ret = recv(sLocal, (char *)pszBuf, wMaxBytes, 0);
	if (ret == -1)
	{
		ret = WSAGetLastError();
		//ShowWSAErrorMsg(ret);
		return -20;
	}
	if (ret<0) return -22;
	if (ret == 0) return -23;

#ifdef DEBUG_TCP_PRINT
	int i;
	for(i=0;i<ret;i++) PRINTK("%02X ",pszBuf[i]);
#endif

	*rBytes = (WORD)ret;
	return ret;
}

/*-------------------------------------------------------------------------
Function:		CTcpTransfer.ShowWSAErrorMsg
Created:		2016-08-23 10:17:30
Author:		Xin Hongwei(xinhw@huahongjt.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	CTcpTransfer::ShowWSAErrorMsg(int nErrNo)
{
	char   buff[128];   
	memset(buff,0x00,128);

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,(void*)FORMAT_MESSAGE_FROM_STRING,nErrNo,0,buff,128,0);   
	MessageBox(NULL,buff,"Error",MB_OK);
	//printf("\n����%s",buff);

	return;
}

