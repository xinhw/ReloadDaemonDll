/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/16 17:02:01

    Reversion:
        
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
Function:		ClsCommand.ClsCommand
Created:		2018-07-16 17:02:07
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
ClsCommand::ClsCommand()
{

	m_ptransfer = NULL;
	init();

}

/*-------------------------------------------------------------------------
Function:		ClsCommand.ClsCommand
Created:		2018-07-16 17:02:11
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
ClsCommand::ClsCommand(CTcpTransfer *pt)
{
	m_ptransfer = pt;
	init();
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.init
Created:		2018-07-16 17:02:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void ClsCommand::init()
{
	memset(m_strBankID,0x00,20);
	m_wRemainCount = 0x0000;
	memset(m_strAgentCode,0x00,7);

	memset(&m_ph,0x00,sizeof(PACKAGEHEADER));

	strcpy(m_ph.strMessageNote,"BANK");
	m_ph.wTransType = 0x0000;
	strcpy(m_ph.strBankCode,getAgentCode());
	m_ph.dwReqLogId = 0;
	m_ph.dwDataLen = 0;
	CMisc::getDateTime(m_ph.strTransTime);
	memset(m_ph.pszMAC,0x00,4);
	m_ph.bCompressFlag=m_ph.bRFU=0x00;

	m_dwTimeout = 3;
}


/*-------------------------------------------------------------------------
Function:		ClsCommand.~ClsCommand
Created:		2018-07-16 17:02:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
ClsCommand::~ClsCommand()
{
}
	
/*-------------------------------------------------------------------------
Function:		ClsCommand.getAgentCode
Created:		2018-07-16 17:02:21
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
char	*ClsCommand::getAgentCode()
{
	return m_strAgentCode;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.setAgentCode
Created:		2018-07-16 17:02:24
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	ClsCommand::setAgentCode(char *s)
{
	if(NULL==s) return;
	
	memset(m_strAgentCode,0x00,7);
	if(strlen(s)>6) 
	{
		memcpy(m_strAgentCode,s,6);
	}
	else
	{
		strcpy(m_strAgentCode,s);
	}
	return;
}


/*-------------------------------------------------------------------------
Function:		ClsCommand.getBankID
Created:		2018-07-16 17:02:27
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
char	*ClsCommand::getBankID()
{
	return m_strBankID;
}



/*-------------------------------------------------------------------------
Function:		ClsCommand.setBankID
Created:		2018-07-16 17:02:30
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	ClsCommand::setBankID(char *s)
{
	if(NULL==s) return;
	
	memset(m_strBankID,0x00,20);
	if(strlen(s)>19) 
	{
		memcpy(m_strBankID,s,19);
	}
	else
	{
		strcpy(m_strBankID,s);
	}
	return;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.getHeader
Created:		2018-07-16 17:02:33
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
PPACKAGEHEADER ClsCommand::getHeader()
{
	return &m_ph;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.getRemainCount
Created:		2018-07-16 17:02:36
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
WORD	ClsCommand::getRemainCount()
{
	return m_wRemainCount;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.updateHeader
Created:		2018-07-16 17:02:39
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	ClsCommand::updateHeader(WORD wType,DWORD wDataLen)
{
	strcpy(m_ph.strMessageNote,"BANK");
	m_ph.wTransType = wType;
	strcpy(m_ph.strBankCode,getAgentCode());
	m_ph.dwReqLogId++;
	m_ph.dwDataLen = wDataLen;
	CMisc::getDateTime(m_ph.strTransTime);
	memset(m_ph.pszMAC,0x00,4);
	m_ph.bCompressFlag=m_ph.bRFU=0x00;	

	return;
}
/*-------------------------------------------------------------------------
Function:		ClsCommand.send_recv
Created:		2018-07-16 17:02:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::send_recv(WORD wTransType,
				  WORD	wLenIn,UCHAR	*pszBufIn,
				  WORD	*wLenOut,UCHAR	*pszBufOut,UINT	nWaitMaxTime)
{
	int				ret;
	BYTE			szBuf[256];
	WORD			nLen,wSize;
	CProtocol		pt;

	//	清空输入缓冲
	while(1)
	{
		ret = m_ptransfer->s_recv(&nLen,szBuf,sizeof(szBuf),100);
		if(ret!=sizeof(szBuf)) break;
	}


	updateHeader(wTransType,wLenIn);

	memset(szBuf,0x00,256);
	ret = pt.combine_a_block(&m_ph,wLenIn,pszBufIn,nLen,szBuf);	
	if(ret)
	{
		PRINTK("\n报文组织失败！:%d",ret);
		return ret;
	}

#ifdef DEBUG_TCP_PRINT
	PRINTK("\nSEND:");
#endif

	ret = m_ptransfer->s_send(nLen, szBuf);
	if (ret)
	{
		PRINTK("\n报文发送失败：%d",ret);
		return ret;
	}

#ifdef DEBUG_TCP_PRINT
	PRINTK("\nRECV:");
#endif

	memset(szBuf,0x00,256);
	ret = m_ptransfer->s_recv(&nLen,szBuf,NHEADDATALEN,m_dwTimeout*1000);
	if(ret<NHEADDATALEN)
	{
		PRINTK("\n报文接收失败1：%d",ret);
		return ret;
	}

	wSize = 0x0000;
	memset(&m_ph,0x00,sizeof(PACKAGEHEADER));
	ret = pt.parse_a_block(nLen,szBuf,&m_ph,wSize,pszBufOut);
	if(ret)
	{
		PRINTK("\n报文头解析出错！");
		return ret;
	}

	memset(szBuf,0x00,256);
	nLen = 0;
	ret = m_ptransfer->s_recv(&nLen,pszBufOut,m_ph.dwDataLen,m_dwTimeout*1000);
	if(ret<m_ph.dwDataLen)
	{
		PRINTK("\n报文接收失败2：%d",ret);
		return ret;
	}

	/*	这里可以比较发送和返回*/
	*wLenOut = m_ph.dwDataLen;

	return 0;
}



//3.1	操作员签到认证1031（一期不实现）
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1031
Created:		2018-07-16 17:02:46
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1031(BYTE *szOperatorNo)
{
	int ret;
	WORD nLen;
	BYTE szBuf[32];

	nLen = 0;
	memset(szBuf,0x00,32);
	
	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2. 操作员编号
	memcpy(szBuf+nLen,szOperatorNo,6);
	nLen = nLen + 6;

	ret = send_recv(0x1031,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;

	m_wRemainCount = szBuf[2];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[3];

	return 0;
}

//3.2	充资认证1032
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1032
Created:		2018-07-16 17:02:49
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1032(BYTE bVer,BYTE *szAPPID,
							BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
							BYTE *szMAC1,
							BYTE *szMAC2)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);
	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	IC卡伪随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	电子存折或电子钱包联机交易序号 	B	4
	CMisc::Int2Bytes(wSeqNo,szBuf+nLen);
	nLen = nLen + 4;
	
	//5	交易金额	B	4
	CMisc::Int2Bytes(nAmount,szBuf+nLen);
	nLen = nLen + 4;

	//6	交易类型标识	B	1
	szBuf[nLen] = bTransFlag;
	nLen++;

	//7	终端机编号	B	6
	memcpy(szBuf+nLen,szDeviceNo,6);
	nLen = nLen + 6;

	//8	主机交易日期	B	4
	//9	主机交易时间	B	3
	memcpy(szBuf+nLen,szDateTime,7);
	nLen = nLen + 7;
	
	//10	MAC1	B	4
	memcpy(szBuf+nLen,szMAC1,4);
	nLen = nLen +4;

	//11	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	//12	交易前余额 B 4
	CMisc::Int2Bytes(dwRemain,szBuf+nLen);
	nLen = nLen + 4;



	ret = send_recv(0x1032,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC2,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}


//3.3	CPU卡二次发行认证1033
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1033
Created:		2018-07-16 17:02:54
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1033(BYTE bVer,BYTE *szAPPID,
							BYTE *szRnd,
							BYTE bFileType,
							BYTE bCmdLen,
							BYTE *szCmd,
							BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[256];

	nLen = 0;
	memset(szBuf,0x00,256);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	IC卡伪随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	文件类别	B	1
	szBuf[nLen] = bFileType;
	nLen++;

	//5	数据长度	B	1
	szBuf[nLen] = bCmdLen;
	nLen++;
	
	//6	文件信息数据	B	L
	memcpy(szBuf+nLen,szCmd,bCmdLen);
	nLen=nLen+bCmdLen;

	//7	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//8	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1033,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}


//3.4	OBU二次发行认证1034
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1034
Created:		2018-07-16 17:02:58
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1034(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[256];

	nLen = 0;
	memset(szBuf,0x00,256);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2 OBU合同号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	文件类别	B	1
	szBuf[nLen] = bFileType;
	nLen++;

	//5	数据长度	B	1
	szBuf[nLen] = bCmdLen;
	nLen++;
	
	//6	文件信息数据	B	L
	memcpy(szBuf+nLen,szCmd,bCmdLen);
	nLen=nLen+bCmdLen;

	//7	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//8	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1034,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}


//3.5	修改储值卡有效期认证1035
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1035
Created:		2018-07-16 17:03:02
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1035(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	数据长度	B	1
	szBuf[nLen] = bCmdLen;
	nLen++;
	
	//5	文件信息数据	B	L
	memcpy(szBuf+nLen,szCmd,bCmdLen);
	nLen=nLen+bCmdLen;

	//6	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//7	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1035,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}


//3.6	消费认证 1036
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1036
Created:		2018-07-16 17:03:06
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1036(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
					BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);
	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	IC卡伪随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	电子存折或电子钱包联机交易序号 	B	4
	CMisc::Int2Bytes(wSeqNo,szBuf+nLen);
	nLen = nLen + 4;
	
	//5	终端交易序号	B	2	必填	最右两个字节
	szBuf[nLen] = (BYTE)((nAuditNo>>8)&0xff);
	szBuf[nLen+1] = (BYTE)(nAuditNo&0xff);
	nLen = nLen + 2;

	//	6	交易前余额	B	4	必填	交易前余额
	CMisc::Int2Bytes(nRemain,szBuf+nLen);
	nLen = nLen + 4;

	//7	交易金额	B	4
	CMisc::Int2Bytes(nAmount,szBuf+nLen);
	nLen = nLen + 4;

	//8	交易类型标识	B	1
	szBuf[nLen] = bTransFlag;
	nLen++;

	//9	终端机编号	B	6
	memcpy(szBuf+nLen,szDeviceNo,6);
	nLen = nLen + 6;

	//10	主机交易日期	B	4
	//11	主机交易时间	B	3
	memcpy(szBuf+nLen,szDateTime,7);
	nLen = nLen + 7;
	
	//12	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//13	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1036,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;


	//1	返回代码	US	2	必填	0     成功其他  失败
	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;

	//2	主机交易日期	B	4	必填	ETC系统日期
	//3	主机交易时间	B	3	必填	ETC系统时间
	memcpy(szDateTime,szBuf+2,7);

	//4	MAC1	B	4	必填	用于执行消费
	memcpy(szMAC,szBuf+9,4);

	//m_wRemainCount = szBuf[6];
	//m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}

//3.7	消费TAC验证 1037
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1037
Created:		2018-07-16 17:03:11
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1037(BYTE bVer,BYTE *szAPPID,
					DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD nAuditNo,BYTE *szDateTime,BYTE *szTAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	交易金额	B	4
	CMisc::Int2Bytes(nAmount,szBuf+nLen);
	nLen = nLen + 4;

	//4	交易类型标识	B	1
	szBuf[nLen] = bTransFlag;
	nLen++;

	//5	终端机编号	B	6
	memcpy(szBuf+nLen,szDeviceNo,6);
	nLen = nLen + 6;

	//6	终端交易序号	B	4
	CMisc::Int2Bytes(nAuditNo,szBuf+nLen);
	nLen = nLen + 4;

	//7	终端交易时间日期	B	7
	memcpy(szBuf+nLen,szDateTime,7);
	nLen = nLen + 7;

	//8	交易TAC码	B	4
	memcpy(szBuf+nLen,szTAC,4);
	nLen = nLen + 4;
	
	//9	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1037,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;


	return 0;
}

//3.8	拆卸标志修改1038
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1038
Created:		2018-07-16 17:03:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1038(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	OBU合同号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	数据长度	B	1
	szBuf[nLen] = bCmdLen;
	nLen++;
	
	//5	拆卸标志数据	B	L
	memcpy(szBuf+nLen,szCmd,bCmdLen);
	nLen=nLen+bCmdLen;

	//6	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//7	合同版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1038,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}

//3.9	PIN解锁认证1039
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1039
Created:		2018-07-16 17:03:19
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1039(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	数据长度	B	1
	szBuf[nLen] = bCmdLen;
	nLen++;
	
	//5	文件信息数据	B	L
	memcpy(szBuf+nLen,szCmd,bCmdLen);
	nLen=nLen+bCmdLen;

	//6	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//7	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1039,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.getKey
Created:		2018-07-16 17:03:22
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::getKey(WORD wType,BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	密钥编号	B	1
	szBuf[nLen] = bKeyNo;
	nLen++;

	//4	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//5	卡片版本号	B	1
	szBuf[nLen] = bVer;
	nLen++;

	//6	随机数	B	8
	memcpy(szBuf+nLen,szRnd,8);
	nLen = nLen + 8;
	
	//7	命令头	B	5
	memcpy(szBuf+nLen,szAPDU,5);
	nLen = nLen + 5;

	//8	密钥头长度	B	1
	szBuf[nLen] = bKeyHeaderLen;
	nLen++;

	//9	密钥头	B	L
	memcpy(szBuf+nLen,szKeyHeader,bKeyHeaderLen);
	nLen=nLen+bKeyHeaderLen;

	//10	保护密钥分散因子	B	8
	memcpy(szBuf+nLen,szPDID,8);
	nLen = nLen + 8;

	ret = send_recv(wType,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;

	nLen = szBuf[2];

	memcpy(szEncKey,szBuf+3,nLen);
	nLen = nLen + 3;
	
	memcpy(szMAC,szBuf+nLen,4);
	nLen = nLen + 4;

	m_wRemainCount = szBuf[nLen];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[nLen+1];
	nLen = nLen + 2;

	return 0;
}


//3.10	CPU卡密钥获取1040
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1040
Created:		2018-07-16 17:03:22
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1040(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC)
{
	int ret;
	
	ret = getKey(0x1040,bVer,szAPPID,bKeyNo,szRnd,szAPDU,bKeyHeaderLen,szKeyHeader,szPDID,szEncKey,szMAC);
	
	return ret;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1042
Created:		2018-07-16 17:03:22
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1042(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC)
{
	int ret;
	
	ret = getKey(0x1042,bVer,szAPPID,bKeyNo,szRnd,szAPDU,bKeyHeaderLen,szKeyHeader,szPDID,szEncKey,szMAC);
	
	return ret;
}

//3.11	CPU卡二次发行补充文件认证1041
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1041
Created:		2018-07-16 17:03:26
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1041(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC)
{
	int ret;
	WORD nLen;
	BYTE szBuf[512];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	用户卡应用序列号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	随机数	B	4
	memcpy(szBuf+nLen,szRnd,4);
	nLen = nLen + 4;

	//4	数据长度	B	1
	szBuf[nLen] = bCmdLen;
	nLen++;
	
	//5	文件信息数据	B	L
	memcpy(szBuf+nLen,szCmd,bCmdLen);
	nLen=nLen+bCmdLen;

	//6	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	//7	卡片版本号	B	1
	szBuf[nLen]=bVer;
	nLen++;

	ret = send_recv(0x1041,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szMAC,szBuf+2,4);

	m_wRemainCount = szBuf[6];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[7];

	return 0;
}


//3.12	计算通行卡的KeyA密钥 2011
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_2011
Created:		2018-07-16 17:03:29
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_2011(BYTE *szCityCode,BYTE *szSerialNo,BYTE *szAuditNo,BYTE *szCardMAC,BYTE bKeyNum,BYTE *szSectorID,
					BYTE *szKey)
{
	int ret;
	WORD nLen;
	BYTE szBuf[64];

	nLen = 0;
	memset(szBuf,0x00,64);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	城市代码	B	2
	memcpy(szBuf+nLen,szCityCode,2);
	nLen = nLen + 2;

	//3	卡唯一号	B	4
	memcpy(szBuf+nLen,szSerialNo,4);
	nLen = nLen + 4;

	//4	发行流水号	B	4
	memcpy(szBuf+nLen,szAuditNo,4);
	nLen = nLen + 4;

	//5	卡认证码	B	4
	memcpy(szBuf+nLen,szCardMAC,4);
	nLen = nLen + 4;

	//6	计算密钥个数	B	1
	szBuf[nLen] = bKeyNum;

	//7	扇区的特征码	B	L
	memcpy(szBuf+nLen,szSectorID,bKeyNum);
	nLen = nLen + bKeyNum;

	//8	认证MAC	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen +4;

	ret = send_recv(0x2011,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	memcpy(szKey,szBuf+2,bKeyNum*6);

	return 0;
}

//4.14	OBU车辆信息文件解密 1043
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1043
Created:		2018-08-27 10:52:54
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1043(BYTE bVer,BYTE *szAPPID,BYTE bKeyIndex,BYTE bInLen,BYTE *szEncData,
							BYTE *bOutLen,BYTE *szData)
{
	int ret;
	WORD nLen;
	BYTE szBuf[256];

	nLen = 0;
	memset(szBuf,0x00,256);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	OBU合同号	B	8
	memcpy(szBuf+nLen,szAPPID,8);
	nLen = nLen + 8;

	//3	加密密钥索引	B	1
	szBuf[nLen] = bKeyIndex;
	nLen++;

	//4	文件密文长度	B	1
	szBuf[nLen] = bInLen;
	nLen++;

	//5	文件密文数据	B	L
	memcpy(szBuf+nLen,szEncData,bInLen);
	nLen = nLen + bInLen;

	//6	RFU	B	4
	memset(szBuf+nLen,0x00,4);
	nLen = nLen + 4;

	//7	合同版本	B	1
	szBuf[nLen] = bVer;
	nLen++;

	ret = send_recv(0x1043,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;

	
	//1	返回代码	US	2		0     成功其他  失败
	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;
	
	//2	明文数据长度	B	1		
	nLen = szBuf[2];
	*bOutLen = nLen;

	//3	车辆信息文件明文	B	L
	memcpy(szData,szBuf+3,nLen);

	//4	剩余交易次数	US	2		
	m_wRemainCount = szBuf[3+nLen];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[4+nLen];

	return 0;
}


//4.15	PSAM卡授权 1044
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1044
Created:		2018-08-27 10:52:49
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1044(BYTE *szSAMNo,BYTE *szRnd,
				DWORD dwRoadID,char *strRoadName,
				DWORD dwStationID, char *strStationName,BYTE bStationType,
				BYTE bLaneType,BYTE bLaneID,
				BYTE *bAPDULen,BYTE *szAPDU,char *strListNo)
{
	int ret;
	WORD nLen,n;
	BYTE szBuf[256];

	nLen = 0;
	memset(szBuf,0x00,256);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	PSAM卡号	B	10	
	memcpy(szBuf+nLen,szSAMNo,10);
	nLen = nLen + 10;

	//3	随机数	B	8	
	memcpy(szBuf+nLen,szRnd,8);
	nLen = nLen + 8;

	//4	路段编码	B	4	
	CMisc::Int2Bytes(dwRoadID,szBuf+nLen);
	nLen = nLen + 4;

	//5	路段名称	ASC	50	
	n = strlen(strRoadName);
	if(n>50) n = 50;
	memcpy(szBuf+nLen,strRoadName,n);
	nLen = nLen + 50;

	//6	收费站编码	B	4	
	CMisc::Int2Bytes(dwStationID,szBuf+nLen);
	nLen = nLen + 4;

	//7	收费站名称	ASC	50	
	n = strlen(strStationName);
	if(n>50) n = 50;
	memcpy(szBuf+nLen,strStationName,n);
	nLen = nLen + 50;

	//8	收费站类型	B	1	
	szBuf[nLen] = bStationType;
	nLen++;

	//9	车道类型	B	1	
	szBuf[nLen] = bLaneType;
	nLen++;

	//10	车道号	B	1	
	szBuf[nLen] = bLaneID;
	nLen++;

	ret = send_recv(0x1044,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;
	if(nLen<52) return -24;

	//1	返回代码	US	2	
	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;

	//2	认证命令长度	B	1	
	nLen = szBuf[2];
	*bAPDULen = nLen;

	//3	认证命令	B	L	
	memcpy(szAPDU,szBuf+3,nLen);
	//4	中心业务流水号	A	36		listNo
	memcpy(strListNo,szBuf+3+nLen,36);

	//5	剩余交易次数	US	2	
	m_wRemainCount = szBuf[39+nLen];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[40+nLen];

	return 0;
}




/*-------------------------------------------------------------------------
Function:		ClsCommand.setWaitTimeout
Created:		2018-08-27 10:22:52
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	ClsCommand::setWaitTimeout(DWORD dwtm)
{
	m_dwTimeout = dwtm;
}

/*-------------------------------------------------------------------------
Function:		ClsCommand.getWaitTimeout
Created:		2018-08-27 10:22:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
DWORD	ClsCommand::getWaitTimeout()
{
	return m_dwTimeout;
}









//4.16	PSAM卡在线授权确认 1045
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1045
Created:		2018-08-27 10:22:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1045(BYTE *szSAMNo,char *strListNo,
				WORD wSW1SW2,BYTE bResult)
{
	int ret;
	WORD nLen;
	BYTE szBuf[256];

	nLen = 0;
	memset(szBuf,0x00,256);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	PSAM卡号	B	10	
	memcpy(szBuf+nLen,szSAMNo,10);
	nLen = nLen + 10;

	//3	申请时中心业务流水号	A	36		申请时的listNo
	memcpy(szBuf+nLen,strListNo,36);
	nLen = nLen + 36;

	//4	指令结果	B	2	
	szBuf[nLen] = (BYTE)(wSW1SW2>>8);
	szBuf[nLen+1] = (BYTE)(wSW1SW2&0xff);
	nLen = nLen + 2;

	//5	指令结果码	B	1	
	szBuf[nLen] = bResult;
	nLen++;

	ret = send_recv(0x1045,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;
	if(nLen<4) return -24;

	//1	返回代码	US	2	
	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;

	//2	剩余交易次数	US	2	
	m_wRemainCount = szBuf[2];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[3];


	return 0;
}

//4.17	PSAM卡在线签到 1046
/*-------------------------------------------------------------------------
Function:		ClsCommand.cmd_1046
Created:		2018-08-27 10:22:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int		ClsCommand::cmd_1046(BYTE *szSAMNo,BYTE *szTerminalNo,
				DWORD dwRoadID,char *strRoadName,
				DWORD dwStationID, char *strStationName,BYTE bStationType,
				BYTE bLaneType,BYTE bLaneID,
				BYTE *szTerminalTime,
				char *strListNo)
{
	int ret;
	WORD nLen,n;
	BYTE szBuf[256];

	nLen = 0;
	memset(szBuf,0x00,256);

	//1	银行网点编码	B	19
	memcpy(szBuf+nLen,m_strBankID,19);
	nLen = nLen + 19;

	//2	PSAM卡号	B	10	
	memcpy(szBuf+nLen,szSAMNo,10);
	nLen = nLen + 10;

	//3	终端号	B	6		PSAM MF下0016文件1~6字节。
	memcpy(szBuf+nLen,szTerminalNo,6);
	nLen = nLen + 6;

	//4	路段编码	B	4	
	CMisc::Int2Bytes(dwRoadID,szBuf+nLen);
	nLen = nLen + 4;

	//5	路段名称	ASC	50	
	n = strlen(strRoadName);
	if(n>50) n = 50;
	memcpy(szBuf+nLen,strRoadName,n);
	nLen = nLen + 50;

	//6	收费站编码	B	4	
	CMisc::Int2Bytes(dwStationID,szBuf+nLen);
	nLen = nLen + 4;

	//7	收费站名称	ASC	50	
	n = strlen(strStationName);
	if(n>50) n = 50;
	memcpy(szBuf+nLen,strStationName,n);
	nLen = nLen + 50;

	//8	收费站类型	B	1	
	szBuf[nLen] = bStationType;
	nLen++;

	//9	车道类型	B	1	
	szBuf[nLen] = bLaneType;
	nLen++;

	//10	车道号	B	1	
	szBuf[nLen] = bLaneID;
	nLen++;

	//11	终端日期时间	B	7		BCD码日期时间
	memcpy(szBuf+nLen,szTerminalTime,7);
	nLen = nLen + 7;

	ret = send_recv(0x1046,nLen,szBuf,&nLen,szBuf);
	if(ret) return ret;
	if(nLen<4) return -24;

	//1	返回代码	US	2	
	ret = szBuf[0];
	ret = ret * 0x100 + szBuf[1];
	if(ret) return ret;

	//2	剩余交易次数	US	2	
	m_wRemainCount = szBuf[2];
	m_wRemainCount = m_wRemainCount *0x100 + szBuf[3];


	return 0;
}
