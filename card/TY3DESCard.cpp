/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/08/06 12:37:59

    Reversion:
        
-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------
Function:		CTY3DESCard.CTY3DESCard
Created:		2018-07-24 16:12:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTY3DESCard::CTY3DESCard()
{
	m_pCmd=NULL;
	m_pReader=NULL;

	memset(m_szPKey,0x00,16);
	memcpy(m_szPKey,"\x41\x42\x43\x53\x58\x53\x51\x54\x54\x59\x43\x4F\x53\x31\x2E\x30",16);
}

/*-------------------------------------------------------------------------
Function:		CTY3DESCard.CTY3DESCard
Created:		2018-07-24 16:12:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTY3DESCard::CTY3DESCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;

	memset(m_szPKey,0x00,16);
	memcpy(m_szPKey,"\x41\x42\x43\x53\x58\x53\x51\x54\x54\x59\x43\x4F\x53\x31\x2E\x30",16);
}

/*-------------------------------------------------------------------------
Function:		CTY3DESCard.~CTY3DESCard
Created:		2018-07-24 16:12:46
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTY3DESCard::~CTY3DESCard()
{

}

/*-------------------------------------------------------------------------
Function:		CTY3DESCard.ty_load_key
Created:		2018-07-24 16:12:49
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTY3DESCard::ty_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader)
{
	int		ret;
	BYTE	szCmd[128],szMAC[8],sRnd[8],szBuf[32],szEncKey[32];
	BYTE	szSesKey[16],szKey[16];
	char	strCmd[512],strResp[128];
	BYTE	bLen = 3;
	int		i;

	//	保护密钥
	memset(szKey,0x00,16);
	if(m_wNetworkID==0x6301)		//	青海
	{
		memcpy(szKey,"\xC3\xA6\xDE\x06\x3F\xA7\x0E\x58\x0E\x37\x11\xA0\x77\x11\xFB\x84",16);
	}

	memset(szSesKey,0x00,16);
	WatchDiversity(szKey,szAPPID,szSesKey,TRUE);

	//	取随机数
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret)
	{
		sprintf(strCmd,"选择随机数失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,8);
	CMisc::StringToByte(strResp,sRnd);	

	memset(szCmd,0x00,128);
	memset(szMAC,0x00,8);

	//	远程取密钥
	ret = m_pCmd->cmd_1040(0x40,szAPPID,
					bKeyNo,
					sRnd,
					(BYTE *)"\x84\xD4\x01\x00\x24",
					bLen,szKeyHeader,
					szAPPID,
					szCmd+32,
					szMAC);
	if(ret)
	{
		PRINTK("\n1040获取Key[%d]失败:%d",bKeyNo,ret);
		return ret;
	}

	
	PRINTK("\n1.");
	for(i=0;i<32;i++) PRINTK("%02X",szCmd[32+i]);
	
	des3_decode(szSesKey,szCmd+32,szCmd);
	des3_decode(szSesKey,szCmd+40,szCmd+8);
	des3_decode(szSesKey,szCmd+48,szCmd+16);
	des3_decode(szSesKey,szCmd+56,szCmd+24);

	PRINTK("\n2.");
	for(i=0;i<32;i++) PRINTK("%02X",szCmd[i]);
	
	memcpy(szKey,szCmd+1+bLen,16);

	memset(szBuf,0x00,32);
	szBuf[0] = 0x10+bKeyHeaderLen;
	memcpy(szBuf+1,szKeyHeader,bKeyHeaderLen);
	memcpy(szBuf+1+bKeyHeaderLen,szKey,16);
	memcpy(szBuf+17+bKeyHeaderLen,(BYTE *)"\x80\x00\x00\x00\x00\x00\x00\x00",8-(1+bKeyHeaderLen)%8);
	
	PRINTK("\n3.");
	for(i=0;i<32;i++) PRINTK("%02X",m_szPKey[i]);
	
	memset(szEncKey,0x00,32);
	des3_encode(m_szPKey,szBuf,szEncKey);
	des3_encode(m_szPKey,szBuf+8,szEncKey+8);
	des3_encode(m_szPKey,szBuf+16,szEncKey+16);
	des3_encode(m_szPKey,szBuf+24,szEncKey+24);

	PRINTK("\n4.");
	for(i=0;i<32;i++) PRINTK("%02X",szEncKey[i]);

	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x84\xD4\x01\x00\x24",5);
	memcpy(szCmd+5,szEncKey,32);

	CmdWatchCalMac(37,szCmd,sRnd,m_szPKey,szCmd+37,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,41,strCmd);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"装载密钥失败，返回:%04X",ret);
		return ret;
	}
	return 0;
}



//	初始化
/*-------------------------------------------------------------------------
Function:		CTY3DESCard.init
Created:		2018-07-24 16:12:53
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTY3DESCard::init(BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	sBuf[128];

	//	MF下的密钥
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,8,(BYTE *)"\x01\x01\x00\x05\x0f\x0F\x0F\x33","SM4 卡片维护密钥 DAMK_DF01",(BYTE *)"\x01\x00"},
		{0x05,8,(BYTE *)"\x00\x01\x00\x08\x0f\x0A\x0F\x33","卡片主控密钥 MK_DF01",(BYTE *)"\x01\x00"},
		{0,0,NULL,NULL}
	};

	CARDKEY arr_adf_keys[] = 
	{
		{0x08,8,(BYTE *)"\x01\x01\x00\x05\x0f\x0F\x0F\x33","应用维护密钥 DAMK_DF01",(BYTE *)"\x01\x00"},
		
		{0x03,8,(BYTE *)"\x01\x01\x00\x08\x0f\x04\x0F\x33","外部认证子密钥1 UK1_DF01",(BYTE *)"\x01\x00"},
		{0x10,8,(BYTE *)"\x00\x01\x00\x09\x0f\x0F\x44\x33","内部认证子密钥1 IK1_DF01",(BYTE *)"\x01\x00"},
		{0x01,8,(BYTE *)"\x01\x01\x00\x00\x0F\x0F\x0f\xff","消费子密钥1 DPK1",(BYTE *)"\x01\x00"},
		{0x02,8,(BYTE *)"\x02\x01\x00\x00\x0F\x0f\x0F\xFF","消费子密钥2 DPK2",(BYTE *)"\x01\x00"},
		{0x04,8,(BYTE *)"\x00\x01\x00\x02\x0F\x00\x0F\xFF","TAC子密钥1 DTK1",(BYTE *)"\x01\x00"},

		{0x09,8,(BYTE *)"\x01\x01\x00\x01\x0F\x0F\x0F\xFF","圈存子密钥1 DLK1",(BYTE *)"\x01\x00"},
		{0x0A,8,(BYTE *)"\x02\x01\x00\x01\x0F\x0F\x0F\xFF","圈存子密钥2 DLK2",(BYTE *)"\x01\x00"},
		{0x0B,8,(BYTE *)"\x00\x00\x00\x06\x0f\x0F\x0F\x33","应用PIN解锁子密钥 DPUK_DF01",(BYTE *)"\x01\x00"},
		{0x0C,8,(BYTE *)"\x01\x00\x00\x07\x0f\x0F\x0F\x33","应用PIN重装子密钥 DPRK_DF01",(BYTE *)"\x01\x00"},
		
		{0x07,8,(BYTE *)"\x00\x01\x00\x08\x0f\x04\x0F\x33","应用主控密钥 MK_DF01",(BYTE *)"\x01\x00"},

		{0,0,NULL,NULL}
	};

	//	卡片版本号和应用序列号
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);
	
	m_wNetworkID = elf15[10];
	m_wNetworkID = m_wNetworkID*0x100 + elf15[11];

	//	选择1PAY.SYS.DDF01
	callbackMessage("选择1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1.0 选择1001
	callbackMessage("选择1001目录 ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000021001",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	取随机数 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"选择随机数失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	加密
	memset(sBuf,0x00,64);

	des3_encode(m_szPKey,sRnd,sBuf);

	//	外部认证
	memset(strCmd,0x00,512);
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"外部认证失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	1.1 写0015文件 
	callbackMessage("写0015文件");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"选择随机数失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);


	memset(sBuf,0x00,128);
	memcpy(sBuf,"\x04\xd6\x95\x00\x2F",5);
	memcpy(sBuf+5,elf15,43);

	CmdWatchCalMac(48,sBuf,sRnd,m_szPKey,sBuf+48,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(sBuf,52,strCmd);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	
	//	1.2 替换ADF应用下密钥
	callbackMessage("装载ADF下其他密钥");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = ty_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	2.0 选择3F00
	callbackMessage("选择1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	取随机数 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"选择随机数失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	加密
	memset(sBuf,0x00,64);
	des3_encode(m_szPKey,sRnd,sBuf);

	//	外部认证
	memset(strCmd,0x00,512);
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"外部认证失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	//	2.1 写0016文件
	callbackMessage("写0016文件");

	//	取随机数
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"选择随机数失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	计算MAC
	memset(sBuf,0x00,128);
	memcpy(sBuf,"\x04\xd6\x96\x00\x3B",5);
	memset(sBuf+5,0xff,55);

	CmdWatchCalMac(60,sBuf,sRnd,m_szPKey,sBuf+60,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(sBuf,64,strCmd);

	//	更新0016
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	2.2 替换MF下的所有密钥
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = ty_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	建立结束
	callbackMessage("卡片建立结束");

	return 0;
}



//	卡片清除
/*-------------------------------------------------------------------------
Function:		CTY3DESCard.clear
Created:		2018-07-24 16:13:00
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTY3DESCard::clear(BYTE *elf15)
{
	return -1;
}


int CTY3DESCard::preInit(WORD wDFID,BYTE *elf15)
{
	return 0;
}
