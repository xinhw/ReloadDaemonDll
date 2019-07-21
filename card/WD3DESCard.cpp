
CWD3DESCard::CWD3DESCard()
{
	m_pCmd = NULL;
	m_pReader = NULL;

	memcpy(m_szPKey,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);
}

CWD3DESCard::CWD3DESCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;

	memcpy(m_szPKey,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);
}

CWD3DESCard::~CWD3DESCard()
{
}


int CWD3DESCard::wd_load_key(BYTE bVer,
							 BYTE *szAPPID,
							 BYTE *szDID,
							 BYTE bKeyNo,
							 BYTE bKeyHeaderLen,BYTE *szKeyHeader,
							 BYTE *szKey)
{
	int		i,ret;
	BYTE	szCmd[128],szMAC[8],szRnd[8];
	BYTE	szSesKey[16];


	memset(szKey,0x00,16);

	memset(szSesKey,0x00,16);
	WatchDiversity(szKey,szAPPID,szSesKey,TRUE);

	memset(szMAC,0x00,8);
	memset(szRnd,0xFF,8);

	memset(szCmd,0x00,128);

	ret = m_pCmd->cmd_1040(0x40,szAPPID,
					bKeyNo,
					szRnd,
					(BYTE *)"\x80\xD4\x01\x01\x15",
					bKeyHeaderLen,szKeyHeader+1,
					szAPPID,
					szCmd+32,
					szMAC);
	if(ret)
	{
		PRINTK("\n1040获取Key[%d]失败:%d",bKeyNo,ret);
		return ret;
	}

	des3_decode(szSesKey,szCmd+32,szCmd);
	des3_decode(szSesKey,szCmd+40,szCmd+8);
	des3_decode(szSesKey,szCmd+48,szCmd+16);

	PRINTK("\n");
	for(i=0;i<32;i++) PRINTK("%02X",szCmd[i]);

	memcpy(szKey,szCmd+1+bKeyHeaderLen,16);

	return wd_secure_update_key(szKeyHeader[1],szKeyHeader[0],szKey);
}


int CWD3DESCard::wd_secure_update_key(BYTE bKeyType,BYTE bKeyVer,BYTE *szKey)
{
	int		ret,i;
	char	strCmd[512],strResp[128];
	BYTE	szCmd[128];
	BYTE	szBuf[32],szEncKey[32];
	BYTE	sRnd[8];

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


	PRINTK("PK:");
	for(i=0;i<16;i++) PRINTK("%02X",m_szPKey[i]);

	memset(szBuf,0x00,32);
	szBuf[0] = 0x10;
	memcpy(szBuf+1,szKey,16);
	memcpy(szBuf+17,"\x80\x00\x00\x00\x00\x00\x00",7);

	memset(szEncKey,0x00,32);
	des3_encode(m_szPKey,szBuf,szEncKey);
	des3_encode(m_szPKey,szBuf+8,szEncKey+8);
	des3_encode(m_szPKey,szBuf+16,szEncKey+16);

	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x84\xD4\x00\x00\x1C",5);
	szCmd[2] = bKeyType;
	szCmd[3] = bKeyVer;
	memcpy(szCmd+5,szEncKey,24);

	CmdWatchCalMac(29,szCmd,sRnd,m_szPKey,szCmd+29,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,33,strCmd);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"装载密钥失败，返回:%04X",ret);
		return ret;
	}
	return 0;
}



//	初始化
int CWD3DESCard::init(BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	szCMK[16],szAMK[16];
	BYTE	sBuf[128];
	BYTE	INIT_KEY[16];

	memcpy(INIT_KEY,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);

	//	MF下的密钥	KeyHeader的第一个字节是P2
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,5,(BYTE *)"\x00\x36\xF0\xF0\xFF\x33","卡片维护密钥 DAMK_CMK"},

		{0,0,NULL,NULL}
	};

	CARDKEY arr_adf_ack[] = 
	{
		{0x07,5,(BYTE *)"\x00\x39\xF0\xF0\xAA\x33","应用主控密钥 MK_DF01"},
		{0,0,NULL,NULL}
	};

	CARDKEY arr_adf_keys[] = 
	{
		{0x03,5,(BYTE *)"\x01\x39\xF0\xF0\x02\x33","外部认证子密钥 UK_DF01"},
		{0x10,5,(BYTE *)"\x00\x30\xF0\xF0\x01\x00","内部认证子密钥 IK_DF01"},
		{0x01,5,(BYTE *)"\x01\x3E\xF0\xF0\x01\x00","消费子密钥1 DPK1"},
		{0x02,5,(BYTE *)"\x02\x3E\xF0\xF0\x01\x00","消费子密钥2 DPK2"},
		{0x04,5,(BYTE *)"\x00\x34\xF0\xF0\x01\x00","TAC子密钥 DTK"},

		{0x09,5,(BYTE *)"\x01\x3F\xF0\xF0\x01\x00","圈存子密钥1 DLK1"},
		{0x0A,5,(BYTE *)"\x02\x3F\xF0\xF0\x01\x00","圈存子密钥2 DLK2"},
		{0x0B,5,(BYTE *)"\x00\x37\xF0\xF0\xFF\x33","应用PIN解锁子密钥 DPUK_DF01"},
		{0x0C,5,(BYTE *)"\x00\x38\xF0\xF0\xFF\x33","应用PIN重装子密钥 DPRK_DF01"},

		{0,0,NULL,NULL}
	};

	//	应用维护密钥
	CARDKEY arr_adf_amk[] = 
	{
		{0x08,5,(BYTE *)"\x00\x36\xF0\xF0\xFF\x33","应用维护密钥 DAMK_DF01"},
		{0,0,NULL,NULL}
	};

	
	//	卡片版本号和应用序列号
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	//	选择1PAY.SYS.DDF01
	callbackMessage("选择1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A404000E315041592E5359532E4444463031",strResp);
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
	des3_encode(INIT_KEY,sRnd,sBuf);

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

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1.1 装载MF下的其他密钥
	memcpy(m_szPKey,INIT_KEY,16);
	
	memset(szCMK,0x00,16);
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szCMK);
		if(ret) return ret;
	}

	//	1.2 写0016文件
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

	CmdWatchCalMac(60,sBuf,sRnd,szCMK,sBuf+60,true);

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
	///////////////////////////////////////////////////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 选择1001
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
	
	//	2.1 替换ADF应用下的其他密钥
	callbackMessage("装载ADF下其他密钥");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szAMK);
		if(ret) return ret;
	}

	//	2.2 装载维护密钥
	memset(szAMK,0x00,16);
	for(pkey=arr_adf_amk;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szAMK);
		if(ret) return ret;
	}

	//	2.3 写0015文件 
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

	CmdWatchCalMac(48,sBuf,sRnd,szAMK,sBuf+48,true);

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

	//	2.4 替换应用主控密钥
	callbackMessage("替换ADF应用主控密钥");
	memset(szCMK,0x00,16);
	for(pkey=arr_adf_ack;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szCMK);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	选择3F00
	callbackMessage("选择1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A404000E315041592E5359532E4444463031",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	3.1 替换（更新）卡片主控密钥
	callbackMessage("装载（替换）卡片主控密钥");

	memset(sBuf,0x00,64);
	WatchDiversity(sBuf,szAPPID,szCMK,TRUE);

	ret = wd_secure_update_key(0x39,0x00,szCMK);
	if(ret) 
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);	
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	建立ADF结束
	callbackMessage("建立ADF结束");
	//	建立结束
	callbackMessage("卡片建立结束");

	return 0;
}


//	卡片清除
int CWD3DESCard::clear(BYTE *elf15)
{
	int		ret;
	BYTE	szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	sBuf[128];
	BYTE	INIT_KEY[16];

	memcpy(INIT_KEY,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);


	//	卡片版本号和应用序列号
	memcpy(szAPPID,elf15+12,8);

	//	选择1PAY.SYS.DDF01
	callbackMessage("选择1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A404000E315041592E5359532E4444463031",strResp);
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
	des3_encode(INIT_KEY,sRnd,sBuf);

	//	外部认证
	memset(strCmd,0x00,512);
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(!ret) goto endl;


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
	WatchDiversity(sBuf,szAPPID,INIT_KEY,TRUE);
	
	memset(sBuf,0x00,64);
	des3_encode(INIT_KEY,sRnd,sBuf);

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

endl:
	//	删除卡片结构
	callbackMessage("删除卡片结构");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("800E000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// 建立密钥文件
	callbackMessage("建立密钥文件");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00000073F003C01AAFFFF",strResp);
	if(ret) 
 	{
		sprintf(strCmd,"失败，返回:%04X",ret);
 		callbackMessage(strCmd);
 		return ret;
 	}
 
	memcpy(m_szPKey,INIT_KEY,16);
 	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	装载卡片主控密钥
	callbackMessage("装载卡片主控密钥");
 
	memset(strCmd,0x00,128);
	memcpy(strCmd,"80D4010015F9F0F0AA33",20);

	memcpy(INIT_KEY,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);
	CMisc::ByteToString(INIT_KEY,16,strCmd+20);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
 	if(ret) 
 	{
 		sprintf(strCmd,"失败，返回:%04X",ret);
 		callbackMessage(strCmd);	
 		return ret;
 	}


	return ret;
}
