/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/24 16:11:59

    Reversion:
        
-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------
Function:		CCPUCardBase.CCPUCardBase
Created:		2018-07-24 16:12:03
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCPUCardBase::CCPUCardBase()
{
	m_pCmd= NULL;
	m_pReader = NULL;
}

/*-------------------------------------------------------------------------
Function:		CCPUCardBase.CCPUCardBase
Created:		2018-07-24 16:12:08
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCPUCardBase::CCPUCardBase(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;
}

/*-------------------------------------------------------------------------
Function:		CCPUCardBase.~CCPUCardBase
Created:		2018-07-24 16:12:11
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCPUCardBase::~CCPUCardBase()
{

}



//	初始化
/*-------------------------------------------------------------------------
Function:		CCPUCardBase.init
Created:		2018-07-24 16:12:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::init(BYTE *elf15)
{
	return 0;
}

//	个人化
/*-------------------------------------------------------------------------
Function:		CCPUCardBase.personalize
Created:		2018-07-24 16:12:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::personalize(BYTE *elf15,BYTE *elf16)
{
	return 0;
}

//	卡片清除
/*-------------------------------------------------------------------------
Function:		CCPUCardBase.clear
Created:		2018-07-24 16:12:21
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::clear(BYTE *elf15)
{
	return 0;
}


bool CCPUCardBase::validation()
{
	if(NULL==m_pReader)
	{
		PRINTK("\n读卡器未初始化");
		return false;
	}
	/*
	if(NULL==m_pCmd)
	{
		PRINTK("\n未连接到前置");
		return false;
	}
	*/

	return true;
}

/*-------------------------------------------------------------------------
Function:		CCPUCardBase.rats
Created:		2018-07-24 16:12:24
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::rats(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS)
{
	int ret;
	char strats[256],strsno[10];

	if(!validation()) return -1;

	memset(strats,0x00,256);
	ret = m_pReader->Initialize(strsno,strats);
	if(0==ret)
	{
		memcpy(szSNO,strsno,4);
		/*
		CMisc::StringToByte(strats,szATS);
		bATSLen = strlen(strats)/2;
		*/
	}

	return ret;
}



/*-------------------------------------------------------------------------
Function:		CCPUCardBase.readCard
Created:		2018-07-24 16:13:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::readCard(BYTE *elf15,BYTE *elf16,DWORD &dwRemain)
{
	int ret;
	char strresp[256];
	BYTE szbuf[32];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 读取0016文件：	持卡人基本数据文件
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00B0960037",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf16);

	//	2. 选择1001 ADF
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	3. 读取0015文件：	卡发行基本数据文件
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00B0950032",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf15);

	//	4. 读取卡片余额
	memset(strresp,0x00,256);
	ret = preader->RunCmd("805C000204",strresp);
	if(ret) return ret;
	memset(szbuf,0x00,32);
	CMisc::StringToByte(strresp,szbuf);

	CMisc::Bytes2Int(szbuf,(int *)&dwRemain);

	return 0;
}

/*-------------------------------------------------------------------------
Function:		CCPUCardBase.readRecord
Created:		2018-07-24 16:13:46
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::readRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec)
{
	int ret;
	char strresp[256],strcmd[11];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	2. 选择1001 ADF
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	1. 读取记录文件
	memset(strcmd,0x00,11);
	sprintf(strcmd,"00B2%02X%02X%02X",bNo,(BYTE)((bFileID>>3)&0x04),bLen);
	
	memset(strresp,0x00,256);
	ret = preader->RunCmd(strcmd,strresp);
	if(ret) return ret;

	CMisc::StringToByte(strresp,szRec);


	return 0;
}

/*-------------------------------------------------------------------------
Function:		CCPUCardBase.credit
Created:		2018-07-24 16:13:50
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::credit(BYTE bVer,BYTE *szAPPID,BYTE *szDeviceNo,DWORD dwAmount,
			WORD &wSeqNo,
			BYTE *szTransTime,
			BYTE *szTAC)
{
	int		ret,i;
	char	strCmd[128],strResp[64];
	BYTE	szBuf[32],szRnd[4],szMAC1[4],szMAC2[4],szDateTime[7];
	DWORD	dwRemain;

	if(!validation()) return -1;

	
	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd("0020000006313233343536",strResp);
	if(ret) return ret;


	//	Initialize for Credit
	memset(strCmd,0x00,128);
	sprintf(strCmd,"805000020B41%08X%02X%02X%02X%02X%02X%02X",dwAmount,
					szDeviceNo[0],szDeviceNo[1],szDeviceNo[2],
					szDeviceNo[3],szDeviceNo[4],szDeviceNo[5]);
	
	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) return ret;

	memset(szBuf,0x00,32);
	CMisc::StringToByte(strResp,szBuf);
	//	余额[4]
	CMisc::Bytes2Int(szBuf,(int *)&dwRemain);
	//	在线交易序号[2]
	wSeqNo = szBuf[4];
	wSeqNo = wSeqNo*0x100 + szBuf[5];
	//[1]
	//[1]
	//	随机数
	memcpy(szRnd,szBuf+8,4);
	//	MAC1
	memcpy(szMAC1,szBuf+12,4);

	CMisc::getBCDDateTime(szDateTime);

	ret = m_pCmd->cmd_1032(bVer,szAPPID,
						szRnd,wSeqNo,dwAmount,0x02,szDeviceNo,szDateTime,
						szMAC2);
	if(ret) return ret;


	memcpy(strCmd,"805200000B",10);
	for(i=0;i<7;i++) sprintf(strCmd+10+2*i,"%02X",szDateTime[i]);
	for(i=0;i<4;i++) sprintf(strCmd+24+2*i,"%02X",szMAC2[i]);

	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) return ret;

	memset(szBuf,0x00,32);
	CMisc::StringToByte(strResp,szBuf);

	memcpy(szTAC,szBuf,4);

	return 0;
}

/*-------------------------------------------------------------------------
Function:		CCPUCardBase.debit
Created:		2018-07-24 16:13:54
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::debit(BYTE bVer,BYTE *szAPPID,BYTE *szDeviceNo,DWORD dwAmount,DWORD dwAuditNo,BYTE *szTransTime,
				WORD &wSeqNo,				
				BYTE *szTAC)
{
	int		ret,i;
	char	strCmd[128],strResp[64];
	BYTE	szBuf[32],szRnd[4],szMAC1[4],szMAC2[4];
	DWORD	dwRemain;

	if(!validation()) return -1;

	//	Initialize for Credit
	memset(strCmd,0x00,128);
	sprintf(strCmd,"805001020B41%08X%02X%02X%02X%02X%02X%02X",dwAmount,
					szDeviceNo[0],szDeviceNo[1],szDeviceNo[2],
					szDeviceNo[3],szDeviceNo[4],szDeviceNo[5]);
	
	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) return ret;

	memset(szBuf,0x00,32);
	CMisc::StringToByte(strResp,szBuf);

	//	余额
	CMisc::Bytes2Int(szBuf,(int *)&dwRemain);
	//	脱机交易序号
	wSeqNo = szBuf[4];
	wSeqNo = wSeqNo*0x100 + szBuf[5];
	//	随机数
	memcpy(szRnd,szBuf+11,4);

	ret = m_pCmd->cmd_1036(bVer,szAPPID,
						szRnd,
						wSeqNo,dwAuditNo,dwRemain,dwAmount,0x06,szDeviceNo,szTransTime,
						szMAC1);
	if(ret) return ret;


	sprintf(strCmd,"805401000F%08X",dwAuditNo);
	for(i=0;i<7;i++) sprintf(strCmd+14+2*i,"%02X",szTransTime[i]);
	for(i=0;i<4;i++) sprintf(strCmd+28+2*i,"%02X",szMAC1[i]);
	strcat(strCmd,"08");

	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) return ret;

	memset(szBuf,0x00,32);
	CMisc::StringToByte(strResp,szBuf);

	memcpy(szTAC,szBuf,4);
	memcpy(szMAC2,szBuf+4,4);

	return 0;
}


