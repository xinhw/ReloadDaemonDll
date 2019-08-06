/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/08/01 14:47:55

    Reversion:
        
-------------------------------------------------------------------------*/

CXH3DESCard::CXH3DESCard()
{
	m_pCmd = NULL;
	m_pReader = NULL;

	memset(m_szPKey,0x00,16);
}

CXH3DESCard::CXH3DESCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;

	memset(m_szPKey,0x00,16);
}

CXH3DESCard::~CXH3DESCard()
{
}


/*-------------------------------------------------------------------------
Function:		CXH3DESCard.xh_load_key
Created:		2019-07-31 15:01:50
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CXH3DESCard::xh_load_key(BYTE bVer,
							 BYTE *szAPPID,
							 BYTE *szDID,
							 BYTE bKeyNo,
							 BYTE bKeyHeaderLen,BYTE *szKeyHeader)
{
	int		ret;
	BYTE	szCmd[128],szMAC[8],sRnd[8],szBuf[32],szEncKey[32];
	BYTE	szSesKey[16],szKey[16];
	char	strCmd[512],strResp[128];
	BYTE	bLen = 3;
	//int		i;

	//	������Կ
	memset(szKey,0x00,16);
	if(m_wNetworkID==0x6301)		//	�ຣ
	{
		memcpy(szKey,"\xC3\xA6\xDE\x06\x3F\xA7\x0E\x58\x0E\x37\x11\xA0\x77\x11\xFB\x84",16);
	}

	memset(szSesKey,0x00,16);
	WatchDiversity(szKey,szAPPID,szSesKey,TRUE);

	//	ȡ�����
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,8);
	CMisc::StringToByte(strResp,sRnd);	

	memset(szCmd,0x00,128);
	memset(szMAC,0x00,8);

	//	Զ��ȡ��Կ
	ret = m_pCmd->cmd_1040(0x40,szAPPID,
					bKeyNo,
					sRnd,
					(BYTE *)"\x84\xD4\x00\x01\x1C",
					bLen,szKeyHeader,
					szAPPID,
					szCmd+32,
					szMAC);
	if(ret)
	{
		PRINTK("\n1040��ȡKey[%d]ʧ��:%d",bKeyNo,ret);
		return ret;
	}

	/*
	PRINTK("\n1.");
	for(i=0;i<32;i++) PRINTK("%02X",szCmd[32+i]);
	*/

	des3_decode(szSesKey,szCmd+32,szCmd);
	des3_decode(szSesKey,szCmd+40,szCmd+8);
	des3_decode(szSesKey,szCmd+48,szCmd+16);

	/*
	PRINTK("\n2.");
	for(i=0;i<32;i++) PRINTK("%02X",szCmd[i]);
	*/

	memcpy(szKey,szCmd+1+bLen,16);

	memset(szBuf,0x00,32);
	szBuf[0] = 0x10+bKeyHeaderLen;
	memcpy(szBuf+1,szKeyHeader,bKeyHeaderLen);
	memcpy(szBuf+1+bKeyHeaderLen,szKey,16);

	/*
	PRINTK("\n3.");
	for(i=0;i<32;i++) PRINTK("%02X",m_szPKey[i]);
	*/

	memset(szEncKey,0x00,32);
	des3_encode(m_szPKey,szBuf,szEncKey);
	des3_encode(m_szPKey,szBuf+8,szEncKey+8);
	des3_encode(m_szPKey,szBuf+16,szEncKey+16);

	/*
	PRINTK("\n4.");
	for(i=0;i<32;i++) PRINTK("%02X",szEncKey[i]);
	*/

	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x84\xD4\x00\x01\x1C",5);
	memcpy(szCmd+5,szEncKey,24);

	CmdWatchCalMac(29,szCmd,sRnd,m_szPKey,szCmd+29,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,33,strCmd);
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
Function:		CXH3DESCard.init
Created:		2019-07-31 15:01:58
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CXH3DESCard::init(BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	szCMK[16],szAMK[16];
	BYTE	sBuf[128];

	//	MF��Կs
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,7,(BYTE *)"\x06\x01\x00\x01\x33\xA0\x00","��Ƭά����Կ DAMK_CMK",NULL},
		{0x05,7,(BYTE *)"\x09\x00\x00\x01\x33\xA0\x01","��Ƭ������Կ MK_DF01",NULL},
		{0,0,NULL,NULL}
	};

	//	ADF��Կs
	CARDKEY arr_adf_keys[] = 
	{
		{0x08,7,(BYTE *)"\x06\x01\x00\x01\x33\xB0\x10","Ӧ��ά����Կ DAMK_DF01",NULL},
		
		{0x03,7,(BYTE *)"\x0B\x01\x00\x01\x33\xB0\x12","�ⲿ��֤����Կ UK_DF01",NULL},
		{0x10,7,(BYTE *)"\x0E\x00\x00\x01\xEE\xB0\x10","�ڲ���֤����Կ IK_DF01",NULL},
		{0x01,7,(BYTE *)"\x01\x01\x00\x01\xEE\xB0\x10","��������Կ1 DPK1",NULL},
		{0x02,7,(BYTE *)"\x01\x02\x00\x01\xEE\xB0\x10","��������Կ2 DPK2",NULL},
		{0x04,7,(BYTE *)"\x03\x00\x00\x01\xEE\xB0\x10","TAC����Կ DTK",NULL},

		{0x09,7,(BYTE *)"\x02\x01\x00\x01\xEE\xB0\x10","Ȧ������Կ1 DLK1",NULL},
		{0x0A,7,(BYTE *)"\x02\x02\x00\x01\xEE\xB0\x10","Ȧ������Կ2 DLK2",NULL},
		{0x0B,7,(BYTE *)"\x11\x00\x00\x01\x33\xB0\x10","Ӧ��PIN��������Կ DPUK_DF01",NULL},
		{0x0C,7,(BYTE *)"\x05\x01\x00\x01\x33\xB0\x10","Ӧ��PIN��װ����Կ DPRK_DF01",NULL},

		{0x07,7,(BYTE *)"\x0B\x00\x00\x01\x33\xA0\x01","Ӧ��������Կ MK_DF01",NULL},

		{0,0,NULL,NULL}
	};

	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	m_wNetworkID = elf15[7];
	m_wNetworkID = m_wNetworkID*0x100 + elf15[8];

	//	ѡ��1PAY.SYS.DDF01
	callbackMessage("ѡ��1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A40000023F00",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1.0 ѡ��1001
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

	memset(szAMK,0x00,16);
	CmdWatchCalMac(48,sBuf,sRnd,szAMK,sBuf+48,true);

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
	
	//	1.2 �滻ADFӦ������Կ
	callbackMessage("װ��ADF��������Կ");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = xh_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	2.0 ѡ��3F00
	callbackMessage("ѡ��1PAY.SYS.DDF01");

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

	memset(szCMK,0x00,16);
	CmdWatchCalMac(60,sBuf,sRnd,szCMK,sBuf+60,true);

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

	//	2.2 �滻MF�µ�������Կ
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = xh_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	��������
	callbackMessage("��Ƭ��������");

	return 0;
}


//	��Ƭ���
/*-------------------------------------------------------------------------
Function:		CXH3DESCard.clear
Created:		2019-07-31 15:02:06
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CXH3DESCard::clear(BYTE *elf15)
{
	callbackMessage("δʵ��");

	return -1;
}


/*-------------------------------------------------------------------------
Function:		CXH3DESCard.preInit
Created:		2019-07-31 15:02:10
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CXH3DESCard::preInit(WORD wDFID,BYTE *elf15)
{
	callbackMessage("δʵ��");

	return -1;
}
