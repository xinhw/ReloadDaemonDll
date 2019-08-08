/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/07/31 15:00:57

    Reversion:
        
-------------------------------------------------------------------------*/



CJD3DESCard::CJD3DESCard()
{
	m_pCmd = NULL;
	m_pReader = NULL;

	memset(m_szPKey,0x20,16);
}

CJD3DESCard::CJD3DESCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;

	memset(m_szPKey,0x20,16);
}

CJD3DESCard::~CJD3DESCard()
{
}


/*-------------------------------------------------------------------------
Function:		CJD3DESCard.jd_load_key_secure
Created:		2019-07-31 15:01:01
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CJD3DESCard::jd_load_key_secure(BYTE bVer,
							 BYTE *szAPPID,
							 BYTE *szDID,
							 BYTE bKeyNo,
							 BYTE bKeyHeaderLen,BYTE *szKeyHeader)
{
	int		i,ret;
	BYTE	szCmd[128],szMAC[8],szRnd[8];
	BYTE	szSesKey[16];
	char	strCmd[512],strResp[128];
	BYTE	szKey[16];
	BYTE	bLen = 3;
	BYTE	szEncKey[32];

	//	解密密钥
	memset(szKey,0x00,16);
	if(m_wNetworkID==0x6301)		//	青海
	{
		memcpy(szKey,"\xC3\xA6\xDE\x06\x3F\xA7\x0E\x58\x0E\x37\x11\xA0\x77\x11\xFB\x84",16);
	}
	WatchDiversity(szKey,szAPPID,szSesKey,TRUE);

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
	memset(szRnd,0x00,8);
	CMisc::StringToByte(strResp,szRnd);
	
	memset(szMAC,0x00,8);
	memset(szCmd,0x00,128);

	ret = m_pCmd->cmd_1040(bVer,szAPPID,
					bKeyNo,
					szRnd,
					(BYTE *)"\x84\xF4\x00\x00\x1C",
					bLen,szKeyHeader,
					szAPPID,
					szCmd+32,
					szMAC);
	if(ret)
	{
		PRINTK("\n1040获取Key[%d]失败:%d",bKeyNo,ret);
		return ret;
	}

	PRINTK("\n0:");
	for(i=0;i<24;i++) PRINTK("%02X",szCmd[32+i]);
	
	//	获取Key
	des3_decode(szSesKey,szCmd+32,szCmd);
	des3_decode(szSesKey,szCmd+40,szCmd+8);
	des3_decode(szSesKey,szCmd+48,szCmd+16);

	PRINTK("\n1:");
	for(i=0;i<24;i++) PRINTK("%02X",szCmd[i]);

	memcpy(szKey,szCmd+bLen+1,16);

	//	保护密钥
	

	memset(szCmd,0x00,128);

	szCmd[0] = 0x11;szCmd[1] = 0xFF;
	memcpy(szCmd+2,szKey,16);
	memcpy(szCmd+18,"\x80\x00\x00\x00\x00\x00",6);

	memset(szEncKey,0x00,32);
	des3_encode(m_szPKey,szCmd,szEncKey);
	des3_encode(m_szPKey,szCmd+8,szEncKey+8);
	des3_encode(m_szPKey,szCmd+16,szEncKey+16);

	PRINTK("\n3:");
	for(i=0;i<24;i++) PRINTK("%02X",szEncKey[i]);

	//	计算MAC
	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x84\xF4\x00\x00\x1C",5);
	memcpy(szCmd+2,szKeyHeader,2);		//	P1,P2
	memcpy(szCmd+5,szEncKey,24);		//	密文[24]

	CmdWatchCalMac(29,szCmd,szRnd,m_szPKey,szCmd+29,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,33,strCmd);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"装载密钥失败，返回:%04X",ret);
		return ret;
	}

	return 0;
}


/*-------------------------------------------------------------------------
Function:		CJD3DESCard.jd_load_key
Created:		2019-07-31 15:01:05
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CJD3DESCard::jd_load_key(BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *szKey)
{
	int		ret;
	char	strCmd[512],strResp[128];

	memset(strCmd,0x00,512);

	strcpy(strCmd,"80F4000018");
	CMisc::ByteToString(szKeyHeader,bKeyHeaderLen,strCmd+10);
	CMisc::ByteToString(szKey,16,strCmd+10+bKeyHeaderLen*2);

	memset(strResp,0x00,128);
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
Function:		CJD3DESCard.init
Created:		2019-07-31 15:01:10
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CJD3DESCard::init(BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;

	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	sBuf[128];


	//	MF下的密钥，KeyHeader是P1和P2
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,2,(BYTE *)"\x32\xB1","卡片维护密钥 DAMK_CMK",(BYTE *)"\x32\xB1"},
		{0x05,2,(BYTE *)"\xB2\xFC","卡片主控密钥 MK_DF01",(BYTE *)"\xB2\xFC"},

		{0,0,NULL,NULL}
	};

	//	ADF下的密钥
	CARDKEY arr_adf_keys[] = 
	{
		{0x08,2,(BYTE *)"\x96\xB2","应用维护密钥 AMK_DF01",(BYTE *)"\x96\xB2"},
		
		{0x10,2,(BYTE *)"\x5A\xF0","内部认证子密钥 IK_DF01",(BYTE *)"\x5A\xF0"},
		{0x03,2,(BYTE *)"\x50\xF1","外部认证子密钥 UK_DF01",(BYTE *)"\x50\xF1"},
		{0x01,2,(BYTE *)"\x91\xB1","消费子密钥1 DPK1",(BYTE *)"\x91\xB1"},
		{0x02,2,(BYTE *)"\x91\xB2","消费子密钥2 DPK2",(BYTE *)"\x91\xB2"},
		{0x09,2,(BYTE *)"\x92\xB1","圈存子密钥1 DLK1",(BYTE *)"\x92\xB1"},
		{0x0A,2,(BYTE *)"\x92\xB2","圈存子密钥2 DLK2",(BYTE *)"\x92\xB2"},
		{0x04,2,(BYTE *)"\x93\xB1","TAC子密钥 DTK",(BYTE *)"\x93\xB1"},
		
		{0x0B,2,(BYTE *)"\x94\xB1","应用PIN解锁子密钥 DPUK_DF01",(BYTE *)"\x94\xB1"},
		{0x0C,2,(BYTE *)"\x95\xB1","应用PIN重装子密钥 DPRK_DF01",(BYTE *)"\x95\xB1"},

		{0x07,2,(BYTE *)"\xB2\xFC","应用主控密钥 ACK_DF01",(BYTE *)"\xB2\xFC"},
		{0,0,NULL,NULL}
	};
	
	//	卡片版本号和应用序列号
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	m_wNetworkID = elf15[10];
	m_wNetworkID = m_wNetworkID*0x100 + elf15[11];

	/////////////////////////////////////////////////////////////////////////////////////
	// 1001目录下的操作
	/////////////////////////////////////////////////////////////////////////////////////
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
	
	//	1.2 认证ACK
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

	//	1.3 替换1001下所有的密钥
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = jd_load_key_secure(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// 3F00
	/////////////////////////////////////////////////////////////////////////////////////
	//	2.0 选择3F00	
	callbackMessage("选择3F00");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
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

	//	2.2 认证CCK
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

	/////////////////////////////////////////////////////////////////////////////////////
	// 2.3 装载MF下的CCK和CMK
	/////////////////////////////////////////////////////////////////////////////////////
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = jd_load_key_secure(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}

	//	建立结束
	callbackMessage("卡片建立结束");
	return 0;
}


//	卡片清除
/*-------------------------------------------------------------------------
Function:		CJD3DESCard.clear
Created:		2019-07-31 15:01:16
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CJD3DESCard::clear(BYTE *elf15)
{
	int		ret;
	BYTE	szAPPID[8];
	
	char	strCmd[512],strResp[128];
	BYTE	INIT_KEY[16],szSesKey[16];
	BYTE	bVer,sRnd[16],sBuf[64];

	//	卡片版本号和应用序列号
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	memset(INIT_KEY,0x00,16);
	memcpy(INIT_KEY,"\xC3\xA6\xDE\x06\x3F\xA7\x0E\x58\x0E\x37\x11\xA0\x77\x11\xFB\x84",16);
	WatchDiversity(INIT_KEY,szAPPID,szSesKey,TRUE);

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
	des3_encode(szSesKey,sRnd,sBuf);

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

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E80000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	return ret;
}


/*-------------------------------------------------------------------------
Function:		CJD3DESCard.preInit
Created:		2019-07-31 15:01:20
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CJD3DESCard::preInit(WORD wDFID,BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;
	WRITEFILEREC *prec;

	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	sBuf[128];
	BYTE	INIT_KEY[16];

	//memcpy(INIT_KEY,"\x15\x87\x04\x40\xBA\x20\x7D\xD9\x27\x35\x88\xDF\xD6\x00\xAD\xC6",16);

	memset(INIT_KEY,0x20,16);

	//	MF下的密钥	KeyHeader的第一个字节是P2
	CARDKEY arr_mf_keys[] = 
	{
		{0x05,8,(BYTE *)"\xB2\xFC\x02\x00\x00\xEE\xFF\x33","卡片主控密钥 MK_DF01",(BYTE *)"\x00\x00"},
		{0x06,8,(BYTE *)"\x32\xB1\x00\x00\x00\xEE\xFF\x33","卡片维护密钥 DAMK_CMK",(BYTE *)"\x00\x00"},

		{0,0,NULL,NULL}
	};


	CARDKEY arr_adf_keys[] = 
	{
		{0x07,8,(BYTE *)"\xB2\xFC\x06\x00\x00\xE6\xFF\x33","应用主控密钥 DAMK_DF01",(BYTE *)"\x00\x00"},
		{0x08,8,(BYTE *)"\x96\xB1\x00\x00\x00\xE6\xFF\x33","应用维护密钥 DAMK_DF01",(BYTE *)"\x00\x00"},
		
		{0x03,8,(BYTE *)"\x50\xF1\x05\x00\x00\xE6\xFF\x33","外部认证子密钥 UK_DF01",(BYTE *)"\x00\x00"},
		{0x10,8,(BYTE *)"\x5A\xF0\x05\x00\x00\xE6\xFF\x33","内部认证子密钥 IK_DF01",(BYTE *)"\x00\x00"},
		{0x01,8,(BYTE *)"\x91\xB1\x00\x00\x00\xE6\xFF\x33 ","消费子密钥1 DPK1",(BYTE *)"\x00\x00"},
		{0x02,8,(BYTE *)"\x91\xB2\x00\x00\x00\xE6\xFF\x33","消费子密钥2 DPK2",(BYTE *)"\x00\x00"},
		{0x04,8,(BYTE *)"\x93\xB1\x00\x00\x00\xE6\xFF\x33","TAC子密钥 DTK",(BYTE *)"\x00\x00"},
		
		{0x09,8,(BYTE *)"\x92\xB1\x00\x00\x00\xE6\xFF\x33","圈存子密钥1 DLK1",(BYTE *)"\x00\x00"},
		{0x0A,8,(BYTE *)"\x92\xB2\x00\x00\x00\xE6\xFF\x33","圈存子密钥2 DLK2",(BYTE *)"\x00\x00"},
		{0x0B,8,(BYTE *)"\x94\xB1\x00\x00\x00\xE6\xFF\x33","应用PIN解锁子密钥 DPUK_DF01",(BYTE *)"\x00\x00"},
		{0x0C,8,(BYTE *)"\x95\xB1\x00\x00\x00\xE6\xFF\x33","应用PIN重装子密钥 DPRK_DF01",(BYTE *)"\x00\x00"},

		{0,0,NULL,NULL}
	};

	//	0019文件的记录格式
	WRITEFILEREC arr0019[] =
	{
		{"01","CC",0x2b,"AA2900"},
		{"02","CC",0x2b,"B12900"},
		{"03","CC",0x2b,"B22900"},
		{"04","CC",0x2b,"B32900"},
		{"05","CC",0x2b,"B42900"},
		{"06","CC",0x2b,"B52900"},
		{"07","CC",0x3f,"C13D00"},
		{"08","CC",0x3f,"C23D00"},
		{"09","CC",0x60,"D15E00"},
		{"0A","CC",0x60,"D25E00"},
		{NULL,NULL,0x00,NULL}
	};

	//	001A和001B文件的记录格式
	WRITEFILEREC arr001A[] =
	{
		{"01","D4",0x1e,"111C00"},
		{"02","D4",0x1e,"121C00"},
		{"03","D4",0x1e,"131C00"},
		{"04","D4",0x1e,"141C00"},
		{"05","D4",0x1e,"151C00"},
		{"06","D4",0x1e,"211C00"},
		{"07","D4",0x1e,"221C00"},
		{"08","D4",0x1e,"231C00"},
		{"09","D4",0x1e,"311C00"},
		{"0A","D4",0x1e,"321C00"},
		{"0B","D4",0x1e,"331C00"},
		{"0C","D4",0x1e,"341C00"},
		{"0D","D4",0x1e,"351C00"},
		{"0E","D4",0x1e,"361C00"},
		{"0F","D4",0x1e,"371C00"},
		{"10","D4",0x1e,"411C00"},
		{"11","D4",0x1e,"421C00"},
		{"12","D4",0x1e,"431C00"},
		{"13","D4",0x1e,"441C00"},
		{"14","D4",0x1e,"451C00"},
		{"15","D4",0x1e,"461C00"},
		{"16","D4",0x1e,"501C00"},
		{"17","D4",0x1e,"511C00"},
		{"18","D4",0x1e,"521C00"},
		{"19","D4",0x1e,"531C00"},
		{"1A","D4",0x1e,"541C00"},
		{"1B","D4",0x1e,"611C00"},
		{"1C","D4",0x1e,"621C00"},
		{"1D","D4",0x1e,"631C00"},
		{"1E","D4",0x1e,"641C00"},
		{"1F","D4",0x1e,"651C00"},
		{"20","D4",0x1e,"711C00"},
		{"21","D4",0x1e,"811C00"},
		{"22","D4",0x1e,"821C00"},

		{NULL,NULL,0x00,NULL}
	};

	
	//	卡片版本号和应用序列号
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	//	选择3F00
	/*
	callbackMessage("选择3F00");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	*/

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E80000023F00",strResp);
	/*
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	*/

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
	des3_encode(INIT_KEY,sRnd,sBuf);

	//	外部认证
	memset(strCmd,0x00,512);
	strcpy(strCmd,"9082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"外部认证失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立MF
	// 80E00100223F0089002E0020000200806F15840E315041592E5359532E4444463031A503880101
	// 80E00100223F0089002E0020000000806F15840E315041592E5359532E4444463031A503880101
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00100223F0089002E0020000000806F15840E315041592E5359532E4444463031A503880101",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立MF 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立密钥文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000038000020CE00002000006E",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立密钥文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// 装载MF下的CCK和CMK
	/////////////////////////////////////////////////////////////////////////////////////
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = jd_load_key(pkey->bHeaderLen,pkey->szKeyHeader,INIT_KEY);
		if(ret) return ret;
	}

	// ;建立0016文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001600000020A0000020160037",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0016文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立0001文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000130000020A000002001003C",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0001文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	// ;建立1001目录
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00100141001E900260020000000006F0784050000000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立1001文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	callbackMessage("选择1001目录 ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A4000C021001",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立密钥文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000038000020E200002000016C",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立密钥文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// 装载ADF下的所有密钥
	/////////////////////////////////////////////////////////////////////////////////////
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = jd_load_key(pkey->bHeaderLen,pkey->szKeyHeader,INIT_KEY);
		if(ret) return ret;
	}

	//	装载PIN密钥
	//80F400001002C10404C023FF33313233343536FFFF
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F400001002C10404C023FF33313233343536FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0015文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	// ;建立0015文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001500000020A0000020150032",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0015文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	写0015文件 
	callbackMessage("写0015文件");

	memset(strCmd,0x00,512);
	memcpy(strCmd,"00d695002B",10);
	CMisc::ByteToString(elf15,43,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立0019文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001930000020A0000020190254",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0019文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;更新0019
	for(prec=arr0019;prec->p1!=NULL;prec++)
	{
		PRINTK("\n更新记录：%s",prec->p1);

		memset(strCmd,0x00,512);
		memcpy(strCmd,"00dcXXccXX",10);
		memcpy(strCmd+4,prec->p1,2);
		memcpy(strCmd+6,prec->p2,2);
		sprintf(strCmd+8,"%02X",prec->bLen);

		memset(strCmd+10,'F',2*prec->bLen);
		memcpy(strCmd+10,prec->applabel,6);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"更新0019文件记录%s 失败，返回:%04X",prec->p1,ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	//	;COS内部数据元文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000DEF0201FFFF232300002002003D",strResp);
	if(ret)
	{
		sprintf(strCmd,"COS内部数据元文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	;更新内部数据元文件，其中包括电子钱包。
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00D682003D0033333333330001000100010001000100010001000000000000000000000000000000000001000100000000000000000000000100010000FFFFFFFF00",strResp);
	if(ret)
	{
		sprintf(strCmd,";更新内部数据元文件，其中包括电子钱包 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立0018文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00182004FF2023000020183217",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0018文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立0012文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00120000002021000020120028",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0012文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立0008文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00080000002021000020080080",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0008文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立0009文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00090000002020000020090200",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立0009文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立001A文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001A30000000800000201A0444",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立001A文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;更新001A
	for(prec=arr001A;prec->p1!=NULL;prec++)
	{
		PRINTK("\n更新记录：%s",prec->p1);

		memset(strCmd,0x00,512);
		memcpy(strCmd,"00DC01D41E",10);
		memcpy(strCmd+4,prec->p1,2);

		memset(strCmd+10,'F',2*prec->bLen);
		memcpy(strCmd+10,prec->applabel,6);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"更新001A文件记录%s 失败，返回:%04X",prec->p1,ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	// ;建立001B文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001B30000020210000201B0444",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立001B文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;更新001B
	for(prec=arr001A;prec->p1!=NULL;prec++)
	{
		PRINTK("\n更新记录：%s",prec->p1);

		memset(strCmd,0x00,512);
		memcpy(strCmd,"00DC01DC1E",10);
		memcpy(strCmd+4,prec->p1,2);

		memset(strCmd+10,'F',2*prec->bLen);
		memcpy(strCmd+10,prec->applabel,6);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"更新001B文件记录%s 失败，返回:%04X",prec->p1,ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	// ;建立001C文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001C00000020210000001C00FF",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立001C文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立001D文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001D00000020210000001D00FF",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立001D文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;建立000A文件
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000A00000020A00000200A00FF",strResp);
	if(ret)
	{
		sprintf(strCmd,"建立000A文件 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//80E00200021001
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00200021001",strResp);
	if(ret)
	{
		sprintf(strCmd,"1001创建完毕 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//00A40000023F00
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"选择3F00 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//80E00200023F00
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00200023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"3F00创建完毕 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;要求取8字节随机数，然后用密钥15870440BA207DD9273588DFD600ADC6进行3DES加密，用9082000008+8字节密文进行OS认证。
	//SYSEXAUTH(15870440BA207DD9273588DFD600ADC6,00)
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
	des3_encode(INIT_KEY,sRnd,sBuf);

	//	外部认证
	memset(strCmd,0x00,512);
	strcpy(strCmd,"9082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"外部认证失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;注意本指令必须执行
	//801400000102
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("801400000102",strResp);
	if(ret)
	{
		sprintf(strCmd,"801400000102 失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	建立结束
	callbackMessage("卡片建立结束");
	return 0;
}



