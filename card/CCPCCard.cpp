/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/08/27 13:38:58

    Reversion:
        
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
Function:		CCPCCard.CCPCCard
Created:		2019-08-27 13:39:02
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCPCCard::CCPCCard()
{
	m_pCmd = NULL;
	m_pReader = NULL;
	memset(m_strListNo,0x00,37);
}

/*-------------------------------------------------------------------------
Function:		CCPCCard.CCPCCard
Created:		2019-08-27 13:39:10
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCPCCard::CCPCCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;
	memset(m_strListNo,0x00,37);
}

/*-------------------------------------------------------------------------
Function:		CCPCCard.~CCPCCard
Created:		2019-08-27 13:39:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CCPCCard::~CCPCCard()
{
}



/*-------------------------------------------------------------------------
Function:		CCPCCard.cpc_load_key
Created:		2019-08-27 13:39:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPCCard::cpc_load_key(BYTE bKeyNo,BYTE *szAPPID,BYTE *bP1P2)
{
	int		ret;
	BYTE	szCmd[128],sRnd[8];
	char	strCmd[512],strResp[128];
	BYTE	bLen = 3;
	//int		i;

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

	//	Զ��ȡ��Կ
	bLen = 0;
	memset(szCmd,0x00,128);

	ret = m_pCmd->cmd_1048(m_strListNo,bKeyNo,sRnd,0x9000,0x00,&bLen,szCmd);
	if(ret)
	{
		PRINTK("\n1048��ȡKey[%d]ʧ��:%d",bKeyNo,ret);
		return ret;
	}

	memset(strCmd,0x00,512);
	sprintf(strCmd,"84D4%02X%02X%02X",bP1P2[0],bP1P2[1],bLen);
	
	CMisc::ByteToString(szCmd,bLen,strCmd+10);

	PRINTK("\n%s",strCmd);

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
Function:		CCPCCard.init
Created:		2019-08-27 13:39:31
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPCCard::init(BYTE *elf15)
{
	int		ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];	
	BYTE	sRnd[16],szBuf[32];
	char	strCmd[512],strResp[128];
	BYTE	szSNO[8];

	//	MF��Կs
	CARDKEY arr_mf_keys[] = 
	{
		{0x01,1,(BYTE *)"\x00","��Ƭ������Կ MK_MF",(BYTE *)"\x01\x00"},
		{0x02,1,(BYTE *)"\x00","��Ƭά����Կ DAMK_MF",(BYTE *)"\x01\xFF"},
		{0,0,NULL,NULL}
	};

	//	ADF��Կs
	CARDKEY arr_adf_keys[] = 
	{
		{0x03,1,(BYTE *)"\x00","Ӧ��������Կ MK_DF01",(BYTE *)"\x01\x00"},
		{0x04,1,(BYTE *)"\x00","Ӧ��ά����Կ DAMK_DF01",(BYTE *)"\x01\xFF"},
		
		{0x05,1,(BYTE *)"\x00","�ڲ���֤����Կ1 IK1_DF01",(BYTE *)"\x01\xFF"},
		{0x06,1,(BYTE *)"\x00","�ڲ���֤����Կ2 IK2_DF01",(BYTE *)"\x01\xFF"},

		{0x07,1,(BYTE *)"\x00","�ⲿ��֤����Կ1 UK1_DF01",(BYTE *)"\x01\xFF"},
		{0x08,1,(BYTE *)"\x00","�ⲿ��֤����Կ2 UK1_DF01",(BYTE *)"\x01\xFF"},
		{0x09,1,(BYTE *)"\x00","�ⲿ��֤����Կ3 UK1_DF01",(BYTE *)"\x01\xFF"},
		{0x0A,1,(BYTE *)"\x00","�ⲿ��֤����Կ4 UK1_DF01",(BYTE *)"\x01\xFF"},



		{0,0,NULL,NULL}
	};

	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[16];
	memcpy(szAPPID,elf15+8,8);

	//	��ȡ��Ψһ��
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F6000304",strResp);
	if(ret)
	{
		sprintf(strCmd,"��ȡ��Ψһ��ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(szSNO,0x00,8);
	CMisc::StringToByte(strResp,szSNO);	

	memset(m_strListNo,0x00,37);
	ret = m_pCmd->cmd_1047(0x51,szSNO,szAPPID,elf15,elf15+17,elf15+21,bVer,m_strListNo);
	if(ret)
	{
		PRINTK("\n1047��ȡListNoʧ��:%d",ret);
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	1.1 ѡ��3F00
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

	//	1.2 ����MF����Կ
	callbackMessage("װ��MF��������Կ");
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = cpc_load_key(pkey->bIndex,szAPPID,pkey->P1P2);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2.0 ѡ��DF01
	callbackMessage("ѡ��DF01Ŀ¼ ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A4000002DF01",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	2.1 �滻ADFӦ������Կ
	callbackMessage("װ��ADF��������Կ");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = cpc_load_key(pkey->bIndex,szAPPID,pkey->P1P2);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	3.0 ѡ��3F00
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

	//	3.1 ����ϵͳ��Ϣ�ļ�
	memset(szBuf,0x00,32);
	ret = m_pCmd->cmd_1049(m_strListNo,sRnd,0x9000,0x00,szBuf);
	if(ret)
	{
		PRINTK("\n1049����MACʧ��:%d",ret);
		return ret;
	}

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);

	strcpy(strCmd,"04D681001D");
	CMisc::ByteToString(elf15,25,strCmd+10);
	CMisc::ByteToString(szBuf,4,strCmd+60);

	PRINTK("\n%s",strCmd);

	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"����EF01�ļ�ʧ�ܣ�����:%04X",ret);
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//	��������
	callbackMessage("��Ƭ��������");

	return 0;
}


//	��Ƭ���

/*-------------------------------------------------------------------------
Function:		CCPCCard.clear
Created:		2019-08-27 13:39:40
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPCCard::clear(BYTE *elf15)
{
	callbackMessage("δʵ��");

	return -1;
}



/*-------------------------------------------------------------------------
Function:		CCPCCard.preInit
Created:		2019-08-27 13:39:49
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPCCard::preInit(WORD wDFID,BYTE *elf15)
{
	callbackMessage("δʵ��");

	return -1;
}
