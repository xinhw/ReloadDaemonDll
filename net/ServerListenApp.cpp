/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/15 09:55:10

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

	PRINTK("\nAPP：客户端服务线程(TCP)运行...");

	CTCPServer		*pts;
	
	//	执行服务器端命令的socket,端口号是LISTEN_PORT_NO
	memset(&slocaladdr,0x00,sizeof(SOCKADDR_IN));
	slocaladdr.sin_family = AF_INET;
	slocaladdr.sin_port =htons(ptp->nportno);
	slocaladdr.sin_addr.s_addr = INADDR_ANY;


	pts = new CTCPServer(&slocaladdr);

	////////////////////////////////////////////////////////////////////////
	PRINTK("\nAPP:server_recv_thread线程开始运行,监听端口%d！",ptp->nportno);
	PRINTK("\nAPP:超时时间段%d秒",ptp->ntimeout);

	ret = pts->HD_Init();
	if(ret)
	{
		PRINTK("\t\t启动失败！");
		goto endl;
	}

	//	设置SOCKET为非阻塞模式。
	fromlen = 1;
	ioctlsocket(pts->m_socket,FIONBIO,(ULONG *)&fromlen);	

	//	轮询接收客户端的链接。
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

		//	2014-03-13：创建线程修改为CreateThread。
		hdl = CreateThread(NULL,NULL,AcceptWorkItem,(PVOID)pcltsock,0,NULL);
		if(hdl==INVALID_HANDLE_VALUE)
		{
			PRINTK("\n来自客户端(TCP)的连接: %s Reject",inet_ntoa(from.sin_addr));
		}
		else
		{
			PRINTK("\n来自客户端(TCP)的连接: %s Wait",inet_ntoa(from.sin_addr));
		}

		InterlockedIncrement(&glConnectNum);

#ifdef THREAD_DISPATCH
		CThreadDispatch::DispatchThread(hdl);
		if(hdl!=INVALID_HANDLE_VALUE) SetThreadPriority(hdl,THREAD_PRIORITY_HIGHEST);
		PRINTK("\n客户端线程数：%d",glConnectNum);
#endif
	}//while(gRun)

endl:

	//PRINTK("\nAPP：客户端服务线程(TCP)退出！");

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

	//	连接的socket和客户端地址
	PCLIENTSOCK		pcltsock = (PCLIENTSOCK)pvContext;
	PACKAGEHEADER	ph;

	CTCPServer		tp(pcltsock->nsocket);
	CProtocol		pt;


	//	解析报文
	PRINTK("\n等待来自客户端(TCP) [%s]的数据:",inet_ntoa(pcltsock->skaddr.sin_addr));

	nIdleTime = GetTickCount()/1000000;

	while(gRun)
	{
		Sleep(1);
		nTick = GetTickCount()/1000000;

		//	长时间没有请求，退出
		if((nTick-nIdleTime)>pcltsock->nWaitTimeOut) 
		{
			PRINTK("\n客户端[socket = %08X]超时退出",pcltsock->nsocket);
			goto endl;
		}


		//	检测是否有输入的包
		ret = tp.socket_waittm(1000,MODE_RECV);
		if(ret==0) continue;
		if(ret==SOCKET_ERROR)
		{		
			nerror = WSAGetLastError();	
			PRINTK("\n错误码：%d",nerror);
			goto endl;
		}

		//	空闲等待
		nIdleTime = nTick;
		memset(pszNetBuff,0x00,HUGE_BUFFER_SIZE);
		
		//取报文头：40字节
		ret = tp.get_recv_buff(NHEADDATALEN,&nLen,pszNetBuff,gMaxWaitTime);
		if(ret!=0) 
		{
			nerror = WSAGetLastError();
			if(-23==ret) ret = 10054;
			if(nerror==0) nerror = ret;
			
			PRINTK("\n错误码：%d",nerror);
			goto endl;
		}

#ifdef	__DEBUG_PRINT__	
		PRINTK("\n请求(%03d):",nLen);
		for(WORD i=0;i<nLen;i++) PRINTK("%02X ",pszNetBuff[i]);
#endif

		//	解析报文
		memset(&ph,0x00,sizeof(PACKAGEHEADER));
		memset(pszData,0x00,MIN_PACKAGE_SIZE);
		ret = pt.parse_a_block(nLen,pszNetBuff,&ph,wSize,pszData);
		if(ret)
		{
			PRINTK("\n解析出错！");
			continue;
		}


		int 	wPacketLen=ph.dwDataLen;
		memset(pszData,0x00,MIN_PACKAGE_SIZE);
		ret = tp.get_recv_buff(wPacketLen,&wSize,pszData,gMaxWaitTime); //取报文数据
		if(ret!=0) 
		{
			nerror = WSAGetLastError();
			if(nerror==0) nerror = ret;
			PRINTK("\n错误码：%d",nerror);
			goto endl;
		}
		if(wPacketLen!=wSize)
		{
			nerror = WSAGetLastError();
			if(nerror==0) nerror = ret;
			PRINTK("\n数据长度不对：%d",nerror);
			goto endl;
		}

#ifdef	__DEBUG_PRINT__	
		for(i=0;i<wSize;i++) PRINTK("%02X ",pszData[i]);
#endif
		//	-----------------------------------------------------------------------------

		//////////////////////////////////////////////////////////////////////////
		//	处理过程		
		//////////////////////////////////////////////////////////////////////////	
		if(NULL==theProc) continue;

		//	typedef int (CALLBACK *processCallback)(PPACKAGEHEADER,WORD,BYTE *,WORD *,BYTE *); 
		//ret = process_request(&ph,wSize,pszData,wSize,pszData);

		wRecv = 0;
		ret = theProc(&ph,wSize,pszData,wRecv,pszData);
		if(ret)
		{
			PRINTK("\n处理错误：%d",ret);
			continue;
		}
		//	-----------------------------------------------------------------------------

		//////////////////////////////////////////////////////////////////////////
		//	给出应答：
		//////////////////////////////////////////////////////////////////////////	
		nLen = 0;
		memset(pszNetBuff,0x00,HUGE_BUFFER_SIZE);
		ret = pt.combine_a_block(&ph,wRecv,pszData,nLen,pszNetBuff);
		if(ret)
		{
			PRINTK("\n应答组包错误：%d",ret);
			continue;
		}

#ifdef	__DEBUG_PRINT__	
		PRINTK("\n应答(%03d):",nLen);
		for(i=0;i<nLen;i++) PRINTK("%02X ",pszNetBuff[i]);
#endif
		ret = tp.s_send(nLen,pszNetBuff);
		if(ret)
		{
			PRINTK("\n应答发送信息出错：%d",ret);
		}
		//	-----------------------------------------------------------------------------
	}

endl:

	InterlockedDecrement(&glConnectNum);

	PRINTK("\n客户端(TCP) [%s]退出",inet_ntoa(pcltsock->skaddr.sin_addr));

	closesocket(pcltsock->nsocket);
	free(pcltsock);

	_endthread();
	return 0;
}


