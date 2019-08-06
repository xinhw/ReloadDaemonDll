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


/*-------------------------------------------------------------------------
Function:		CCPUCardBase.validation
Created:		2018-07-28 10:57:39
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
bool CCPUCardBase::validation()
{
	if(NULL==m_pReader)
	{
		PRINTK("\n读卡器未初始化");
		return false;
	}
	
	if(NULL==m_pCmd)
	{
		PRINTK("\n未连接到前置");
		return false;
	}

	return true;
}

int CCPUCardBase::preInit(WORD wDFID,BYTE *elf15)
{
	return 0;
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
	BYTE strats[256],strsno[10];

	if(!validation()) return -1;

	memset(strats,0x00,256);
	memset(strsno,0x00,10);

	ret = m_pReader->Initialize(strsno,bATSLen,strats);
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
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 读取0016文件：	持卡人基本数据文件
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00B0960037",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf16);

	//	2. 选择1001 ADF
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	3. 读取0015文件：	卡发行基本数据文件
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00B0950032",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf15);

	//	4. 读取卡片余额
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("805C000204",strresp);
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
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 选择1001 ADF
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	2. 读取记录文件
	memset(strcmd,0x00,11);
	sprintf(strcmd,"00B2%02X%02X%02X",bNo,(BYTE)((bFileID>>3)&0x04),bLen);
	
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd(strcmd,strresp);
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
	BYTE	bKeyIndex;

	if(!validation()) return -1;

	bKeyIndex = 0x01;
	if((bVer>>4)==0x05) bKeyIndex = 0x41;
	
	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strResp);
	if(ret) return ret;

	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd("0020000006313233343536",strResp);
	if(ret) return ret;


	//	Initialize for Credit
	memset(strCmd,0x00,128);
	sprintf(strCmd,"805000020B%02X%08X%02X%02X%02X%02X%02X%02X",bKeyIndex,dwAmount,
					szDeviceNo[0],szDeviceNo[1],szDeviceNo[2],
					szDeviceNo[3],szDeviceNo[4],szDeviceNo[5]);
	
	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) return ret;

	memset(szBuf,0x00,32);
	CMisc::StringToByte(strResp,szBuf);
	//	余额		[4]
	CMisc::Bytes2Int(szBuf,(int *)&dwRemain);
	//	在线交易序号	[2]
	wSeqNo = szBuf[4];
	wSeqNo = wSeqNo*0x100 + szBuf[5];
	//	密钥标识	[1]
	//	密钥版本	[1]
	//	随机数		[4]
	memcpy(szRnd,szBuf+8,4);
	//	MAC1		[4]
	memcpy(szMAC1,szBuf+12,4);

	CMisc::getBCDDateTime(szDateTime);

	ret = m_pCmd->cmd_1032(bVer,szAPPID,
						szRnd,wSeqNo,dwAmount,0x02,szDeviceNo,szDateTime,dwRemain,
						szMAC1,
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
	int		ret;
	char	strCmd[128],strResp[64];
	BYTE	szBuf[32],szRnd[4],szMAC1[4],szMAC2[4];
	DWORD	dwRemain;
	BYTE	bKeyIndex;

	if(!validation()) return -1;

	bKeyIndex = 0x01;
	if((bVer>>4)==0x05) bKeyIndex = 0x41;

	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strResp);
	if(ret) return ret;

	//	Initialize for Debit
	memset(strCmd,0x00,128);
	sprintf(strCmd,"805001020B%02X%08X%02X%02X%02X%02X%02X%02X",bKeyIndex,dwAmount,
					szDeviceNo[0],szDeviceNo[1],szDeviceNo[2],
					szDeviceNo[3],szDeviceNo[4],szDeviceNo[5]);
	
	memset(strResp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) return ret;

	memset(szBuf,0x00,32);
	CMisc::StringToByte(strResp,szBuf);

	//电子存折或电子钱包旧余额						4
	CMisc::Bytes2Int(szBuf,(int *)&dwRemain);

	//电子存折或电子钱包脱机交易序号				2
	wSeqNo = szBuf[4];
	wSeqNo = wSeqNo*0x100 + szBuf[5];

	//透支限额										3
	//密钥版本号（DATA中第一字节指定的消费密钥）	1
	//算法标识（DATA中第一字节指定的消费密钥）		1
	//伪随机数（IC卡）								4
	memcpy(szRnd,szBuf+11,4);

	ret = m_pCmd->cmd_1036(bVer,szAPPID,
						szRnd,
						wSeqNo,dwAuditNo,dwRemain,dwAmount,0x06,szDeviceNo,szTransTime,
						szMAC1);
	if(ret) return ret;

	//终端交易序号		4
	//交易日期（终端）	4
	//交易时间（终端）	3
	//MAC1	4
	sprintf(strCmd,"805401000F%08X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
					dwAuditNo,
					szTransTime[0],szTransTime[1],szTransTime[2],szTransTime[3],
					szTransTime[4],szTransTime[5],szTransTime[6],
					szMAC1[0],szMAC1[1],szMAC1[2],szMAC1[3]);

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



/*-------------------------------------------------------------------------
Function:		CCPUCardBase.updateELF0015
Created:		2018-07-27 10:32:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::updateELF0015(BYTE bVer,BYTE *szAPPID,BYTE *szFile0015)
{
	int		ret,i;
	char	strCmd[256],strresp[256];
	BYTE	szCmd[128],szRnd[4],szMAC[4];

	if(!validation()) return -1;

	//	1. 选择1001 ADF
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	2. 取随机数
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	CMisc::StringToByte(strresp,szRnd);
	
	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x04\xD6\x95\x00\x36",5);
	memcpy(szCmd+5,szFile0015,50);

	memset(szMAC,0x00,4);
	ret = m_pCmd->cmd_1033(bVer,szAPPID,szRnd,0x02,55,szCmd,szMAC);
	if(ret) return ret;

	//	3. 更新文件
	strcpy(strCmd,"04D6950036");
	for(i=0;i<50;i++) sprintf(strCmd+10+2*i,"%02X",szFile0015[i]);
	for(i=0;i<4;i++) sprintf(strCmd+110+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}



/*-------------------------------------------------------------------------
Function:		CCPUCardBase.updateELF0016
Created:		2018-07-27 10:32:35
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::updateELF0016(BYTE bVer,BYTE *szAPPID,BYTE *szFile0016)
{
	int		ret,i;
	char	strCmd[256],strresp[64];
	BYTE	szCmd[128],szRnd[4],szMAC[4];

	if(!validation()) return -1;

	//	1. 选择3F00
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	2. 取随机数
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	CMisc::StringToByte(strresp,szRnd);
	
	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x04\xD6\x96\x00\x3B",5);
	memcpy(szCmd+5,szFile0016,55);

	memset(szMAC,0x00,4);
	ret = m_pCmd->cmd_1033(bVer,szAPPID,szRnd,0x01,60,szCmd,szMAC);
	if(ret) return ret;

	//	3. 更新文件
	strcpy(strCmd,"04D696003B");
	for(i=0;i<55;i++) sprintf(strCmd+10+2*i,"%02X",szFile0016[i]);
	for(i=0;i<4;i++) sprintf(strCmd+120+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}


/*-------------------------------------------------------------------------
Function:		CCPUCardBase.updateValidDate
Created:		2018-07-27 10:32:35
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::updateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szNewValidDate)
{
	int		ret,i;
	char	strCmd[256],strresp[256];
	BYTE	szCmd[128],szRnd[4],szMAC[4];

	if(!validation()) return -1;

	//	1. 选择1001 ADF
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	2. 取随机数
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	CMisc::StringToByte(strresp,szRnd);
	
	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x04\xD6\x95\x18\x08",5);
	memcpy(szCmd+5,szNewValidDate,4);

	memset(szMAC,0x00,4);
	ret = m_pCmd->cmd_1035(bVer,szAPPID,szRnd,9,szCmd,szMAC);
	if(ret) return ret;

	//	3. 更新储值卡的有效日期
	strcpy(strCmd,"04D6951808");
	for(i=0;i<4;i++) sprintf(strCmd+10+2*i,"%02X",szNewValidDate[i]);
	for(i=0;i<4;i++) sprintf(strCmd+18+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}




/*-------------------------------------------------------------------------
Function:		CCPUCardBase.updateELF000E
Created:		2018-07-27 10:32:35
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::updateELF000E(BYTE bVer,BYTE *szAPPID,BYTE *szFile000E)
{
	int		ret,i;
	char	strCmd[256],strresp[256];
	BYTE	szCmd[256],szRnd[4],szMAC[4];

	if(!validation()) return -1;

	//	1. 选择1001 ADF
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	2. 取随机数
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	CMisc::StringToByte(strresp,szRnd);
	
	memset(szCmd,0x00,256);
	memcpy(szCmd,"\x04\xD6\x8E\x00\x4A",5);
	memcpy(szCmd+5,szFile000E,70);

	memset(szMAC,0x00,4);
	ret = m_pCmd->cmd_1041(bVer,szAPPID,szRnd,75,szCmd,szMAC);
	if(ret) return ret;

	//	3. 更新000E文件
	strcpy(strCmd,"04D68E004A");
	for(i=0;i<70;i++) sprintf(strCmd+10+2*i,"%02X",szFile000E[i]);
	for(i=0;i<4;i++) sprintf(strCmd+150+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}






/*-------------------------------------------------------------------------
Function:		CCPUCardBase.reloadPIN
Created:		2018-07-28 10:57:24
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::reloadPIN(BYTE bVer,BYTE *szAPPID,BYTE bPINLen,BYTE *szPIN)
{
	int		ret,i;
	char	strCmd[64],strresp[256];
	BYTE	szCmd[32],szRnd[8];

	if(!validation()) return -1;

	//	1. 选择1001 ADF
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	
	memset(szCmd,0x00,32);
	memcpy(szCmd,"\x80\x5E\x00\x41\x00",5);
	szCmd[4] = (BYTE)(4+bPINLen);
	memcpy(szCmd+5,szPIN,bPINLen);

	memset(szRnd,0x00,8);

	ret = m_pCmd->cmd_1039(bVer,szAPPID,szRnd,5+bPINLen,szCmd,szCmd+5+bPINLen);
	if(ret) return ret;

	//	3. RELOAD PIN
	memset(strCmd,0x00,64);
	for(i=0;i<(bPINLen+9);i++) sprintf(strCmd+2*i,"%02X",szCmd[i]);

	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}


/*-------------------------------------------------------------------------
Function:		CCPUCardBase.readAdfFile
Created:		2018-08-04 13:42:43
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPUCardBase::readAdfFile(BYTE bFID,BYTE bOffset, BYTE bLength, BYTE *szFile)
{
	int		ret;
	char	strCmd[64],strresp[256];
	BYTE	bFileID;

	if(!validation()) return -1;

	//	0. 选择3F00
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 选择1001 ADF
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("00A40000021001",strresp);
	if(ret) return ret;

	//	2. 读取二进制文件
	bFileID = (BYTE)(0x80+bFID);

	memset(strCmd,0x00,64);
	sprintf(strCmd,"00B0%02X%02X%02X",bFileID,bOffset,bLength);

	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	CMisc::StringToByte(strresp,szFile);

	return 0;
}