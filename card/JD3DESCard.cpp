/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/07/31 15:00:57

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

	//	������Կ
	memset(szKey,0x00,16);
	if(m_wNetworkID==0x6301)		//	�ຣ
	{
		memcpy(szKey,"\xC3\xA6\xDE\x06\x3F\xA7\x0E\x58\x0E\x37\x11\xA0\x77\x11\xFB\x84",16);
	}
	WatchDiversity(szKey,szAPPID,szSesKey,TRUE);

	//	ȡ����� 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
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
		PRINTK("\n1040��ȡKey[%d]ʧ��:%d",bKeyNo,ret);
		return ret;
	}

	PRINTK("\n0:");
	for(i=0;i<24;i++) PRINTK("%02X",szCmd[32+i]);
	
	//	��ȡKey
	des3_decode(szSesKey,szCmd+32,szCmd);
	des3_decode(szSesKey,szCmd+40,szCmd+8);
	des3_decode(szSesKey,szCmd+48,szCmd+16);

	PRINTK("\n1:");
	for(i=0;i<24;i++) PRINTK("%02X",szCmd[i]);

	memcpy(szKey,szCmd+bLen+1,16);

	//	������Կ
	

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

	//	����MAC
	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x84\xF4\x00\x00\x1C",5);
	memcpy(szCmd+2,szKeyHeader,2);		//	P1,P2
	memcpy(szCmd+5,szEncKey,24);		//	����[24]

	CmdWatchCalMac(29,szCmd,szRnd,m_szPKey,szCmd+29,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,33,strCmd);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"װ����Կʧ�ܣ�����:%04X",ret);
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
		sprintf(strCmd,"װ����Կʧ�ܣ�����:%04X",ret);
		return ret;
	}
	return 0;
}


//	��ʼ��
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


	//	MF�µ���Կ��KeyHeader��P1��P2
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,2,(BYTE *)"\x32\xB1","��Ƭά����Կ DAMK_CMK",(BYTE *)"\x32\xB1"},
		{0x05,2,(BYTE *)"\xB2\xFC","��Ƭ������Կ MK_DF01",(BYTE *)"\xB2\xFC"},

		{0,0,NULL,NULL}
	};

	//	ADF�µ���Կ
	CARDKEY arr_adf_keys[] = 
	{
		{0x08,2,(BYTE *)"\x96\xB2","Ӧ��ά����Կ AMK_DF01",(BYTE *)"\x96\xB2"},
		
		{0x10,2,(BYTE *)"\x5A\xF0","�ڲ���֤����Կ IK_DF01",(BYTE *)"\x5A\xF0"},
		{0x03,2,(BYTE *)"\x50\xF1","�ⲿ��֤����Կ UK_DF01",(BYTE *)"\x50\xF1"},
		{0x01,2,(BYTE *)"\x91\xB1","��������Կ1 DPK1",(BYTE *)"\x91\xB1"},
		{0x02,2,(BYTE *)"\x91\xB2","��������Կ2 DPK2",(BYTE *)"\x91\xB2"},
		{0x09,2,(BYTE *)"\x92\xB1","Ȧ������Կ1 DLK1",(BYTE *)"\x92\xB1"},
		{0x0A,2,(BYTE *)"\x92\xB2","Ȧ������Կ2 DLK2",(BYTE *)"\x92\xB2"},
		{0x04,2,(BYTE *)"\x93\xB1","TAC����Կ DTK",(BYTE *)"\x93\xB1"},
		
		{0x0B,2,(BYTE *)"\x94\xB1","Ӧ��PIN��������Կ DPUK_DF01",(BYTE *)"\x94\xB1"},
		{0x0C,2,(BYTE *)"\x95\xB1","Ӧ��PIN��װ����Կ DPRK_DF01",(BYTE *)"\x95\xB1"},

		{0x07,2,(BYTE *)"\xB2\xFC","Ӧ��������Կ ACK_DF01",(BYTE *)"\xB2\xFC"},
		{0,0,NULL,NULL}
	};
	
	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	m_wNetworkID = elf15[10];
	m_wNetworkID = m_wNetworkID*0x100 + elf15[11];

	/////////////////////////////////////////////////////////////////////////////////////
	// 1001Ŀ¼�µĲ���
	/////////////////////////////////////////////////////////////////////////////////////
	callbackMessage("ѡ��1001Ŀ¼ ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000021001",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	1.1 д0015�ļ� 
	callbackMessage("д0015�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
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
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	
	//	1.2 ��֤ACK
	//	ȡ����� 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	����
	memset(sBuf,0x00,64);
	des3_encode(m_szPKey,sRnd,sBuf);

	//	�ⲿ��֤
	memset(strCmd,0x00,512);
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"�ⲿ��֤ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	1.3 �滻1001�����е���Կ
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = jd_load_key_secure(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// 3F00
	/////////////////////////////////////////////////////////////////////////////////////
	//	2.0 ѡ��3F00	
	callbackMessage("ѡ��3F00");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	
	
	//	2.1 д0016�ļ�
	callbackMessage("д0016�ļ�");

	//	ȡ�����
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	����MAC
	memset(sBuf,0x00,128);
	memcpy(sBuf,"\x04\xd6\x96\x00\x3B",5);
	memset(sBuf+5,0xff,55);

	CmdWatchCalMac(60,sBuf,sRnd,m_szPKey,sBuf+60,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(sBuf,64,strCmd);

	//	����0016
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	2.2 ��֤CCK
	//	ȡ����� 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	����
	memset(sBuf,0x00,64);
	des3_encode(m_szPKey,sRnd,sBuf);

	//	�ⲿ��֤
	memset(strCmd,0x00,512);
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"�ⲿ��֤ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// 2.3 װ��MF�µ�CCK��CMK
	/////////////////////////////////////////////////////////////////////////////////////
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = jd_load_key_secure(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}

	//	��������
	callbackMessage("��Ƭ��������");
	return 0;
}


//	��Ƭ���
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

	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	memset(INIT_KEY,0x00,16);
	memcpy(INIT_KEY,"\xC3\xA6\xDE\x06\x3F\xA7\x0E\x58\x0E\x37\x11\xA0\x77\x11\xFB\x84",16);
	WatchDiversity(INIT_KEY,szAPPID,szSesKey,TRUE);

	//	ȡ����� 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	����
	memset(sBuf,0x00,64);
	des3_encode(szSesKey,sRnd,sBuf);

	//	�ⲿ��֤
	memset(strCmd,0x00,512);
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"�ⲿ��֤ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E80000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
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

	//	MF�µ���Կ	KeyHeader�ĵ�һ���ֽ���P2
	CARDKEY arr_mf_keys[] = 
	{
		{0x05,8,(BYTE *)"\xB2\xFC\x02\x00\x00\xEE\xFF\x33","��Ƭ������Կ MK_DF01",(BYTE *)"\x00\x00"},
		{0x06,8,(BYTE *)"\x32\xB1\x00\x00\x00\xEE\xFF\x33","��Ƭά����Կ DAMK_CMK",(BYTE *)"\x00\x00"},

		{0,0,NULL,NULL}
	};


	CARDKEY arr_adf_keys[] = 
	{
		{0x07,8,(BYTE *)"\xB2\xFC\x06\x00\x00\xE6\xFF\x33","Ӧ��������Կ DAMK_DF01",(BYTE *)"\x00\x00"},
		{0x08,8,(BYTE *)"\x96\xB1\x00\x00\x00\xE6\xFF\x33","Ӧ��ά����Կ DAMK_DF01",(BYTE *)"\x00\x00"},
		
		{0x03,8,(BYTE *)"\x50\xF1\x05\x00\x00\xE6\xFF\x33","�ⲿ��֤����Կ UK_DF01",(BYTE *)"\x00\x00"},
		{0x10,8,(BYTE *)"\x5A\xF0\x05\x00\x00\xE6\xFF\x33","�ڲ���֤����Կ IK_DF01",(BYTE *)"\x00\x00"},
		{0x01,8,(BYTE *)"\x91\xB1\x00\x00\x00\xE6\xFF\x33 ","��������Կ1 DPK1",(BYTE *)"\x00\x00"},
		{0x02,8,(BYTE *)"\x91\xB2\x00\x00\x00\xE6\xFF\x33","��������Կ2 DPK2",(BYTE *)"\x00\x00"},
		{0x04,8,(BYTE *)"\x93\xB1\x00\x00\x00\xE6\xFF\x33","TAC����Կ DTK",(BYTE *)"\x00\x00"},
		
		{0x09,8,(BYTE *)"\x92\xB1\x00\x00\x00\xE6\xFF\x33","Ȧ������Կ1 DLK1",(BYTE *)"\x00\x00"},
		{0x0A,8,(BYTE *)"\x92\xB2\x00\x00\x00\xE6\xFF\x33","Ȧ������Կ2 DLK2",(BYTE *)"\x00\x00"},
		{0x0B,8,(BYTE *)"\x94\xB1\x00\x00\x00\xE6\xFF\x33","Ӧ��PIN��������Կ DPUK_DF01",(BYTE *)"\x00\x00"},
		{0x0C,8,(BYTE *)"\x95\xB1\x00\x00\x00\xE6\xFF\x33","Ӧ��PIN��װ����Կ DPRK_DF01",(BYTE *)"\x00\x00"},

		{0,0,NULL,NULL}
	};

	//	0019�ļ��ļ�¼��ʽ
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

	//	001A��001B�ļ��ļ�¼��ʽ
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

	
	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	//	ѡ��3F00
	/*
	callbackMessage("ѡ��3F00");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
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
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	*/

	//	ȡ����� 
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	����
	memset(sBuf,0x00,64);
	des3_encode(INIT_KEY,sRnd,sBuf);

	//	�ⲿ��֤
	memset(strCmd,0x00,512);
	strcpy(strCmd,"9082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"�ⲿ��֤ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����MF
	// 80E00100223F0089002E0020000200806F15840E315041592E5359532E4444463031A503880101
	// 80E00100223F0089002E0020000000806F15840E315041592E5359532E4444463031A503880101
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00100223F0089002E0020000000806F15840E315041592E5359532E4444463031A503880101",strResp);
	if(ret)
	{
		sprintf(strCmd,"����MF ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;������Կ�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000038000020CE00002000006E",strResp);
	if(ret)
	{
		sprintf(strCmd,"������Կ�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// װ��MF�µ�CCK��CMK
	/////////////////////////////////////////////////////////////////////////////////////
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = jd_load_key(pkey->bHeaderLen,pkey->szKeyHeader,INIT_KEY);
		if(ret) return ret;
	}

	// ;����0016�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001600000020A0000020160037",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0016�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0001�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000130000020A000002001003C",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0001�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	// ;����1001Ŀ¼
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00100141001E900260020000000006F0784050000000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"����1001�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	callbackMessage("ѡ��1001Ŀ¼ ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A4000C021001",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;������Կ�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000038000020E200002000016C",strResp);
	if(ret)
	{
		sprintf(strCmd,"������Կ�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// װ��ADF�µ�������Կ
	/////////////////////////////////////////////////////////////////////////////////////
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = jd_load_key(pkey->bHeaderLen,pkey->szKeyHeader,INIT_KEY);
		if(ret) return ret;
	}

	//	װ��PIN��Կ
	//80F400001002C10404C023FF33313233343536FFFF
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F400001002C10404C023FF33313233343536FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0015�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	// ;����0015�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001500000020A0000020150032",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0015�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	д0015�ļ� 
	callbackMessage("д0015�ļ�");

	memset(strCmd,0x00,512);
	memcpy(strCmd,"00d695002B",10);
	CMisc::ByteToString(elf15,43,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0019�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001930000020A0000020190254",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0019�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0019
	for(prec=arr0019;prec->p1!=NULL;prec++)
	{
		PRINTK("\n���¼�¼��%s",prec->p1);

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
			sprintf(strCmd,"����0019�ļ���¼%s ʧ�ܣ�����:%04X",prec->p1,ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	//	;COS�ڲ�����Ԫ�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000DEF0201FFFF232300002002003D",strResp);
	if(ret)
	{
		sprintf(strCmd,"COS�ڲ�����Ԫ�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	;�����ڲ�����Ԫ�ļ������а�������Ǯ����
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00D682003D0033333333330001000100010001000100010001000000000000000000000000000000000001000100000000000000000000000100010000FFFFFFFF00",strResp);
	if(ret)
	{
		sprintf(strCmd,";�����ڲ�����Ԫ�ļ������а�������Ǯ�� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0018�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00182004FF2023000020183217",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0018�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0012�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00120000002021000020120028",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0012�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0008�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00080000002021000020080080",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0008�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����0009�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D00090000002020000020090200",strResp);
	if(ret)
	{
		sprintf(strCmd,"����0009�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����001A�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001A30000000800000201A0444",strResp);
	if(ret)
	{
		sprintf(strCmd,"����001A�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����001A
	for(prec=arr001A;prec->p1!=NULL;prec++)
	{
		PRINTK("\n���¼�¼��%s",prec->p1);

		memset(strCmd,0x00,512);
		memcpy(strCmd,"00DC01D41E",10);
		memcpy(strCmd+4,prec->p1,2);

		memset(strCmd+10,'F',2*prec->bLen);
		memcpy(strCmd+10,prec->applabel,6);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"����001A�ļ���¼%s ʧ�ܣ�����:%04X",prec->p1,ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	// ;����001B�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001B30000020210000201B0444",strResp);
	if(ret)
	{
		sprintf(strCmd,"����001B�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����001B
	for(prec=arr001A;prec->p1!=NULL;prec++)
	{
		PRINTK("\n���¼�¼��%s",prec->p1);

		memset(strCmd,0x00,512);
		memcpy(strCmd,"00DC01DC1E",10);
		memcpy(strCmd+4,prec->p1,2);

		memset(strCmd+10,'F',2*prec->bLen);
		memcpy(strCmd+10,prec->applabel,6);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"����001B�ļ���¼%s ʧ�ܣ�����:%04X",prec->p1,ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	// ;����001C�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001C00000020210000001C00FF",strResp);
	if(ret)
	{
		sprintf(strCmd,"����001C�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����001D�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D001D00000020210000001D00FF",strResp);
	if(ret)
	{
		sprintf(strCmd,"����001D�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ;����000A�ļ�
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E003000D000A00000020A00000200A00FF",strResp);
	if(ret)
	{
		sprintf(strCmd,"����000A�ļ� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//80E00200021001
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00200021001",strResp);
	if(ret)
	{
		sprintf(strCmd,"1001������� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//00A40000023F00
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ��3F00 ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//80E00200023F00
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00200023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"3F00������� ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;Ҫ��ȡ8�ֽ��������Ȼ������Կ15870440BA207DD9273588DFD600ADC6����3DES���ܣ���9082000008+8�ֽ����Ľ���OS��֤��
	//SYSEXAUTH(15870440BA207DD9273588DFD600ADC6,00)
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000008",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	����
	memset(sBuf,0x00,64);
	des3_encode(INIT_KEY,sRnd,sBuf);

	//	�ⲿ��֤
	memset(strCmd,0x00,512);
	strcpy(strCmd,"9082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"�ⲿ��֤ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;ע�Ȿָ�����ִ��
	//801400000102
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("801400000102",strResp);
	if(ret)
	{
		sprintf(strCmd,"801400000102 ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	��������
	callbackMessage("��Ƭ��������");
	return 0;
}



