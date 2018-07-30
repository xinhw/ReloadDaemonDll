/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/15 09:55:10

    Reversion:
        
-------------------------------------------------------------------------*/
 



/*-------------------------------------------------------------------------
Function:		server_recv_thread
Created:		2018-07-15 09:55:13
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
DWORD	WINAPI server_recv_thread(LPVOID p)
{
	int				ret;
	SOCKADDR_IN		slocaladdr;
	SOCKET			client;
	SOCKADDR_IN		from;
	PCLIENTSOCK		pcltsock = NULL;
	PTHREADPARAM	ptp =(PTHREADPARAM)p;

	HANDLE			hdl=INVALID_HANDLE_VALUE;

	int fromlen = sizeof(from);	

	PRINTK("\nAPP���ͻ��˷����߳�(TCP)����...");

	CTCPServer		*pts;
	
	//	ִ�з������������socket,�˿ں���LISTEN_PORT_NO
	memset(&slocaladdr,0x00,sizeof(SOCKADDR_IN));
	slocaladdr.sin_family = AF_INET;
	slocaladdr.sin_port =htons(ptp->nportno);
	slocaladdr.sin_addr.s_addr = INADDR_ANY;


	pts = new CTCPServer(&slocaladdr);

	////////////////////////////////////////////////////////////////////////
	PRINTK("\nAPP:server_recv_thread�߳̿�ʼ����,�����˿�%d��",ptp->nportno);
	PRINTK("\nAPP:��ʱʱ���%d��",ptp->ntimeout);

	ret = pts->HD_Init();
	if(ret)
	{
		PRINTK("\t\t����ʧ�ܣ�");
		goto endl;
	}

	//	����SOCKETΪ������ģʽ��
	fromlen = 1;
	ioctlsocket(pts->m_socket,FIONBIO,(ULONG *)&fromlen);	

	//	��ѯ���տͻ��˵����ӡ�
	while(gRun)
	{
		Sleep(1);

		if(listen(pts->m_socket,SOMAXCONN)!=0) continue;

		fromlen = sizeof(from);
		client = accept(pts->m_socket,(struct sockaddr *)&from,&fromlen);
		if(client==INVALID_SOCKET) 
		{
			continue;
		}

		pcltsock =(PCLIENTSOCK)malloc(sizeof(CLIENTSOCK));
		pcltsock->nsocket = client;
		memcpy(&pcltsock->skaddr,&from,sizeof(SOCKADDR_IN));

		pcltsock->nWaitTimeOut = ptp->ntimeout;

		//	2014-03-13�������߳��޸�ΪCreateThread��
		hdl = CreateThread(NULL,NULL,AcceptWorkItem,(PVOID)pcltsock,0,NULL);
		if(hdl==INVALID_HANDLE_VALUE)
		{
			PRINTK("\n���Կͻ���(TCP)������: %s Reject",inet_ntoa(from.sin_addr));
		}
		else
		{
			PRINTK("\n���Կͻ���(TCP)������: %s Wait",inet_ntoa(from.sin_addr));
		}

		InterlockedIncrement(&glConnectNum);

#ifdef THREAD_DISPATCH
		CThreadDispatch::DispatchThread(hdl);
		if(hdl!=INVALID_HANDLE_VALUE) SetThreadPriority(hdl,THREAD_PRIORITY_HIGHEST);
		PRINTK("\n�ͻ����߳�����%d",glConnectNum);
#endif
	}//while(gRun)

endl:

	//PRINTK("\nAPP���ͻ��˷����߳�(TCP)�˳���");

	pts->HD_Term();
	free(ptp);
	return 0;
}




/*-------------------------------------------------------------------------
Function:		AcceptWorkItem
Created:		2018-07-15 09:55:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
DWORD	WINAPI	AcceptWorkItem(PVOID pvContext)
{
	int				ret,nerror;
	UINT			nIdleTime = 0;
	UINT			nTick = 0;

	WORD			nLen;
	BYTE			pszNetBuff[HUGE_BUFFER_SIZE];

	WORD			wSize,wRecv;
	BYTE			pszData[MIN_PACKAGE_SIZE];

	//	���ӵ�socket�Ϳͻ��˵�ַ
	PCLIENTSOCK		pcltsock = (PCLIENTSOCK)pvContext;
	PACKAGEHEADER	ph;

	CTCPServer		tp(pcltsock->nsocket);
	CProtocol		pt;


	//	��������
	PRINTK("\n�ȴ����Կͻ���(TCP) [%s]������:",inet_ntoa(pcltsock->skaddr.sin_addr));

	nIdleTime = GetTickCount()/1000000;

	while(gRun)
	{
		Sleep(1);
		nTick = GetTickCount()/1000000;

		//	��ʱ��û�������˳�
		if((nTick-nIdleTime)>pcltsock->nWaitTimeOut) 
		{
			PRINTK("\n�ͻ���[socket = %08X]��ʱ�˳�",pcltsock->nsocket);
			goto endl;
		}


		//	����Ƿ�������İ�
		ret = tp.socket_waittm(1000,MODE_RECV);
		if(ret==0) continue;
		if(ret==SOCKET_ERROR)
		{		
			nerror = WSAGetLastError();	
			PRINTK("\n�����룺%d",nerror);
			goto endl;
		}

		//	���еȴ�
		nIdleTime = nTick;
		memset(pszNetBuff,0x00,HUGE_BUFFER_SIZE);
		
		//ȡ����ͷ��40�ֽ�
		ret = tp.get_recv_buff(NHEADDATALEN,&nLen,pszNetBuff,gMaxWaitTime);
		if(ret!=0) 
		{
			nerror = WSAGetLastError();
			if(-23==ret) ret = 10054;
			if(nerror==0) nerror = ret;
			
			PRINTK("\n�����룺%d",nerror);
			goto endl;
		}

#ifdef	__DEBUG_PRINT__	
		PRINTK("\n����(%03d):",nLen);
		for(WORD i=0;i<nLen;i++) PRINTK("%02X ",pszNetBuff[i]);
#endif

		//	��������
		memset(&ph,0x00,sizeof(PACKAGEHEADER));
		memset(pszData,0x00,MIN_PACKAGE_SIZE);
		ret = pt.parse_a_block(nLen,pszNetBuff,&ph,wSize,pszData);
		if(ret)
		{
			PRINTK("\n��������");
			continue;
		}


		int 	wPacketLen=ph.dwDataLen;
		memset(pszData,0x00,MIN_PACKAGE_SIZE);
		ret = tp.get_recv_buff(wPacketLen,&wSize,pszData,gMaxWaitTime); //ȡ��������
		if(ret!=0) 
		{
			nerror = WSAGetLastError();
			if(nerror==0) nerror = ret;
			PRINTK("\n�����룺%d",nerror);
			goto endl;
		}
		if(wPacketLen!=wSize)
		{
			nerror = WSAGetLastError();
			if(nerror==0) nerror = ret;
			PRINTK("\n���ݳ��Ȳ��ԣ�%d",nerror);
			goto endl;
		}

#ifdef	__DEBUG_PRINT__	
		for(i=0;i<wSize;i++) PRINTK("%02X ",pszData[i]);
#endif
		//	-----------------------------------------------------------------------------

		//////////////////////////////////////////////////////////////////////////
		//	�������		
		//////////////////////////////////////////////////////////////////////////	
		if(NULL==theProc) continue;

		//	typedef int (CALLBACK *processCallback)(PPACKAGEHEADER,WORD,BYTE *,WORD *,BYTE *); 
		//ret = process_request(&ph,wSize,pszData,wSize,pszData);

		wRecv = 0;
		ret = theProc(&ph,wSize,pszData,wRecv,pszData);
		if(ret)
		{
			PRINTK("\n�������%d",ret);
			continue;
		}
		//	-----------------------------------------------------------------------------

		//////////////////////////////////////////////////////////////////////////
		//	����Ӧ��
		//////////////////////////////////////////////////////////////////////////	
		nLen = 0;
		memset(pszNetBuff,0x00,HUGE_BUFFER_SIZE);
		ret = pt.combine_a_block(&ph,wRecv,pszData,nLen,pszNetBuff);
		if(ret)
		{
			PRINTK("\nӦ���������%d",ret);
			continue;
		}

#ifdef	__DEBUG_PRINT__	
		PRINTK("\nӦ��(%03d):",nLen);
		for(i=0;i<nLen;i++) PRINTK("%02X ",pszNetBuff[i]);
#endif
		ret = tp.s_send(nLen,pszNetBuff);
		if(ret)
		{
			PRINTK("\nӦ������Ϣ����%d",ret);
		}
		//	-----------------------------------------------------------------------------
	}

endl:

	InterlockedDecrement(&glConnectNum);

	PRINTK("\n�ͻ���(TCP) [%s]�˳�",inet_ntoa(pcltsock->skaddr.sin_addr));

	closesocket(pcltsock->nsocket);
	free(pcltsock);

	_endthread();
	return 0;
}


