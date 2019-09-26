/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/08/27 13:38:58

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

	//	远程取密钥
	bLen = 0;
	memset(szCmd,0x00,128);

	ret = m_pCmd->cmd_1048(m_strListNo,bKeyNo,sRnd,0x9000,0x00,&bLen,szCmd);
	if(ret)
	{
		PRINTK("\n1048获取Key[%d]失败:%d",bKeyNo,ret);
		return ret;
	}

	memset(strCmd,0x00,512);
	sprintf(strCmd,"84D4%02X%02X%02X",bP1P2[0],bP1P2[1],bLen);
	
	CMisc::ByteToString(szCmd,bLen,strCmd+10);

	PRINTK("\n%s",strCmd);

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

	//	MF密钥s
	CARDKEY arr_mf_keys[] = 
	{
		{0x01,1,(BYTE *)"\x00","卡片主控密钥 MK_MF",(BYTE *)"\x01\x00"},
		{0x02,1,(BYTE *)"\x00","卡片维护密钥 DAMK_MF",(BYTE *)"\x01\xFF"},
		{0,0,NULL,NULL}
	};

	//	ADF密钥s
	CARDKEY arr_adf_keys[] = 
	{
		{0x03,1,(BYTE *)"\x00","应用主控密钥 MK_DF01",(BYTE *)"\x01\x00"},
		{0x04,1,(BYTE *)"\x00","应用维护密钥 DAMK_DF01",(BYTE *)"\x01\xFF"},
		
		{0x05,1,(BYTE *)"\x00","内部认证子密钥1 IK1_DF01",(BYTE *)"\x01\xFF"},
		{0x06,1,(BYTE *)"\x00","内部认证子密钥2 IK2_DF01",(BYTE *)"\x01\xFF"},

		{0x07,1,(BYTE *)"\x00","外部认证子密钥1 UK1_DF01",(BYTE *)"\x01\xFF"},
		{0x08,1,(BYTE *)"\x00","外部认证子密钥2 UK1_DF01",(BYTE *)"\x01\xFF"},
		{0x09,1,(BYTE *)"\x00","外部认证子密钥3 UK1_DF01",(BYTE *)"\x01\xFF"},
		{0x0A,1,(BYTE *)"\x00","外部认证子密钥4 UK1_DF01",(BYTE *)"\x01\xFF"},



		{0,0,NULL,NULL}
	};

	//	卡片版本号和应用序列号
	bVer = elf15[16];
	memcpy(szAPPID,elf15+8,8);

	//	获取卡唯一号
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F6000304",strResp);
	if(ret)
	{
		sprintf(strCmd,"获取卡唯一号失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(szSNO,0x00,8);
	CMisc::StringToByte(strResp,szSNO);	

	memset(m_strListNo,0x00,37);
	ret = m_pCmd->cmd_1047(0x51,szSNO,szAPPID,elf15,elf15+17,elf15+21,bVer,m_strListNo);
	if(ret)
	{
		PRINTK("\n1047获取ListNo失败:%d",ret);
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	1.1 选择3F00
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

	//	1.2 更新MF下密钥
	callbackMessage("装载MF下其他密钥");
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = cpc_load_key(pkey->bIndex,szAPPID,pkey->P1P2);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2.0 选择DF01
	callbackMessage("选择DF01目录 ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A4000002DF01",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	2.1 替换ADF应用下密钥
	callbackMessage("装载ADF下其他密钥");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n装载密钥：%s",pkey->strDesc);
		ret = cpc_load_key(pkey->bIndex,szAPPID,pkey->P1P2);
		if(ret) return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	3.0 选择3F00
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

	//	3.1 更新系统信息文件
	memset(szBuf,0x00,32);
	ret = m_pCmd->cmd_1049(m_strListNo,sRnd,0x9000,0x00,szBuf);
	if(ret)
	{
		PRINTK("\n1049计算MAC失败:%d",ret);
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
		sprintf(strCmd,"更新EF01文件失败，返回:%04X",ret);
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//	建立结束
	callbackMessage("卡片建立结束");

	return 0;
}


//	卡片清除

/*-------------------------------------------------------------------------
Function:		CCPCCard.clear
Created:		2019-08-27 13:39:40
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CCPCCard::clear(BYTE *elf15)
{
	callbackMessage("未实现");

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
	callbackMessage("未实现");

	return -1;
}
