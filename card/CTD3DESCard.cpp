/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/08/06 12:38:35

    Reversion:
        	������3DES��
-------------------------------------------------------------------------*/

CCTD3DESCard::CCTD3DESCard()
{
	m_pCmd = NULL;
	m_pReader = NULL;

	memset(m_szPKey,0x00,16);
}

CCTD3DESCard::CCTD3DESCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;

	memset(m_szPKey,0x00,16);
}

CCTD3DESCard::~CCTD3DESCard()
{
}


/*-------------------------------------------------------------------------
Function:		CCTD3DESCard.ctd_load_key
Created:		2019-07-31 15:01:50
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCTD3DESCard::ctd_load_key(BYTE bVer,
							 BYTE *szAPPID,
							 BYTE *szDID,
							 BYTE bKeyNo,
							 BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *P1P2)
{
	int		ret;
	BYTE	szCmd[128],szMAC[8],sRnd[8],szBuf[32],szEncKey[32];
	BYTE	szSesKey[16],szKey[16];
	char	strCmd[512],strResp[128];
	BYTE	bLen = 3;
	int		i;

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
					(BYTE *)"\x84\xD4\x00\x01\x15",
					bLen,szKeyHeader,
					szAPPID,
					szCmd+32,
					szMAC);
	if(ret)
	{
		PRINTK("\n1040��ȡKey[%d]ʧ��:%d",bKeyNo,ret);
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
	memcpy(szCmd,"\x84\xD4\x00\x01\x24",5);
	memcpy(szCmd+2,P1P2,2);
	memcpy(szCmd+5,szEncKey,32);

	CmdWatchCalMac(37,szCmd,sRnd,m_szPKey,szCmd+37,true);

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,41,strCmd);
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
Function:		CCTD3DESCard.init
Created:		2019-07-31 15:01:58
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCTD3DESCard::init(BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	sBuf[128];

	//	MF��Կs
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,8,(BYTE *)"\x36\xF0\xFF\x01\x00\x00\x33\x10","��Ƭά����Կ DAMK_CMK",(BYTE *)"\xB6\x01"},
		{0x05,8,(BYTE *)"\x39\xF0\xFF\x00\x0F\x00\x33\x10","��Ƭ������Կ MK_DF01",(BYTE *)"\xB9\x00"},
		{0,0,NULL,NULL}
	};

	//	ADF��Կs
	CARDKEY arr_adf_keys[] = 
	{
		{0x08,8,(BYTE *)"\x36\xF0\xFF\x01\x00\x00\x33\x10","Ӧ��ά����Կ DAMK_DF01",(BYTE *)"\xB6\x01"},
		
		{0x03,8,(BYTE *)"\x39\xF0\xFF\x01\x01\x00\x33\x10","�ⲿ��֤����Կ UK_DF01",(BYTE *)"\xB9\x01"},
		{0x10,8,(BYTE *)"\x35\xF0\xFF\x00\x00\x00\x33\x10","�ڲ���֤����Կ IK_DF01",(BYTE *)"\xB5\x00"},
		{0x01,8,(BYTE *)"\x3E\xF0\xFF\x01\x00\x00\x00\x10","��������Կ1 DPK1",(BYTE *)"\xBE\x01"},
		{0x02,8,(BYTE *)"\x3E\xF0\xFF\x02\x00\x00\x00\x10","��������Կ2 DPK2",(BYTE *)"\xBE\x02"},
		{0x04,8,(BYTE *)"\x3A\xF0\xFF\x00\x00\x00\x00\x10","TAC����Կ DTK",(BYTE *)"\xBA\x00"},

		{0x09,8,(BYTE *)"\x3F\x22\xFF\x01\x00\x00\x00\x10","Ȧ������Կ1 DLK1",(BYTE *)"\xBF\x01"},
		{0x0A,8,(BYTE *)"\x3F\x22\xFF\x02\x00\x00\x00\x10","Ȧ������Կ2 DLK2",(BYTE *)"\xBF\x02"},
		{0x0B,8,(BYTE *)"\x37\xF0\xFF\x00\x00\x00\x33\x10","Ӧ��PIN��������Կ DPUK_DF01",(BYTE *)"\xB7\x00"},
		{0x0C,8,(BYTE *)"\x38\xF0\xFF\x01\x00\x00\x33\x10","Ӧ��PIN��װ����Կ DPRK_DF01",(BYTE *)"\xB8\x01"},

		{0x07,8,(BYTE *)"\x39\xF0\xFF\x00\x0F\x00\x33\x10","Ӧ��������Կ MK_DF01",(BYTE *)"\xB9\x00"},

		{0,0,NULL,NULL}
	};

	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);
	
	m_wNetworkID = elf15[10];
	m_wNetworkID = m_wNetworkID*0x100 + elf15[11];

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
	
	//	1.2 �滻ADFӦ������Կ
	callbackMessage("װ��ADF��������Կ");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = ctd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,pkey->P1P2);
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

	//	2.2 �滻MF�µ�������Կ
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = ctd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,pkey->P1P2);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	��������
	callbackMessage("��Ƭ��������");

	return 0;
}


//	��Ƭ���
/*-------------------------------------------------------------------------
Function:		CCTD3DESCard.clear
Created:		2019-07-31 15:02:06
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCTD3DESCard::clear(BYTE *elf15)
{
	callbackMessage("δʵ��");

	return -1;
}


/*-------------------------------------------------------------------------
Function:		CCTD3DESCard.preInit
Created:		2019-07-31 15:02:10
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCTD3DESCard::preInit(WORD wDFID,BYTE *elf15)
{
	callbackMessage("δʵ��");

	return -1;
}
