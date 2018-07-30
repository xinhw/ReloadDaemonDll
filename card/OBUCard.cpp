/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/27 14:07:12

    Reversion:
        
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
Function:		COBUCard.COBUCard
Created:		2018-07-27 14:07:19
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
COBUCard::COBUCard()
{
	m_pCmd=NULL;
	m_pReader=NULL;
}

/*-------------------------------------------------------------------------
Function:		COBUCard.COBUCard
Created:		2018-07-27 14:07:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
COBUCard::COBUCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd=pcmd;
	m_pReader=preader;
}

/*-------------------------------------------------------------------------
Function:		COBUCard.~COBUCard
Created:		2018-07-27 14:07:37
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
COBUCard::~COBUCard()
{
}



/*-------------------------------------------------------------------------
Function:		COBUCard.obu_load_key
Created:		2018-07-28 11:34:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCard::obu_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader)
{
	int		ret;
	BYTE	sRnd[8];
	char	strCmd[128],strResp[32];
	BYTE	szEncKey[32],szMAC[4];

	//	1. 取随机数
	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret) return ret;

	memset(sRnd,0x00,8);
	CMisc::StringToByte(strResp,sRnd);

	//	2. 获取密钥密文和MAC
	ret = m_pCmd->cmd_1042(bVer,szAPPID,
							bKeyNo,
							sRnd,
							(BYTE *)"\x84\xD4\x01\xFF\x24",
							bKeyHeaderLen,szKeyHeader,
							szDID,
							szEncKey,
							szMAC);
	if(ret) return ret;

	//	3. 装载密钥
	memset(strCmd,0x00,128);
	memcpy(strCmd,"84D401FF24",10);
	CMisc::ByteToString(szEncKey,32,strCmd+10);
	CMisc::ByteToString(szMAC,4,strCmd+74);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	
	return ret;
}
//	初始化
/*-------------------------------------------------------------------------
Function:		COBUCard.init
Created:		2018-07-27 14:07:43
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCard::init(BYTE *elf01_mf,BYTE *elf01_adf)
{
	int		ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	szEncKey[32],szMAC[4];

	BYTE	szOBUID[8];


	//	MF下的密钥
	CARDKEY arr_mf_keys[] = 
	{
		{0x02,3,(BYTE *)"\x01\x01\x00","SM4 系统维护密钥 DAMK_MF"},

		{0,0,NULL,NULL}
	};

	//	ADF1下的密钥
	CARDKEY arr_adf_keys[] = 
	{
		{0x04,3,(BYTE *)"\x01\x01\x00","SM4 应用维护密钥 DAMK_DF01"},
		
		{0x05,3,(BYTE *)"\x01\x02\x00","3DES 外部认证密钥1 UK1_DF01"},
		{0x06,3,(BYTE *)"\x01\x03\x00","3DES 应用加密密钥1 RK21_DF01"},
		{0x07,3,(BYTE *)"\x01\x03\x01","3DES 应用加密密钥2 RK22_DF01"},
		{0x08,3,(BYTE *)"\x01\x03\x02","3DES 应用加密密钥3 RK23_DF01"},
			
		{0x09,3,(BYTE *)"\x00\x41\x00","SM4外部认证密钥2 UK2_DF01"},
		{0x0A,3,(BYTE *)"\x01\x42\x00","SM4应用认证密钥 RK3_DF01"},
		{0x0B,3,(BYTE *)"\x01\x43\x40","SM4应用加密密钥4 RK41_DF01"},
		{0x0C,3,(BYTE *)"\x01\x43\x41","SM4应用加密密钥5 RK42_DF01"},
		{0x0D,3,(BYTE *)"\x01\x43\x42","SM4应用加密密钥6 RK43_DF01"},


		{0,0,NULL,NULL}
	};

	//	版本号和应用序列号
	bVer = elf01_mf[9];
	memcpy(szAPPID,elf01_mf+10,8);


	//	获取OBU号：		80F6000304 		返回4字节，记作OBUUID
	callbackMessage("获取OBU号");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F6000304",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(szOBUID,0x00,8);
	CMisc::StringToByte(strResp,szOBUID);
	memcpy(szOBUID+4,szOBUID,4);


	//	用户卡选择3F00:	00A40000023F00
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

	//	替换主控密钥：	84D401001C	1002FE 	000000（密钥头） 	分散因子是OBUUID重复两遍，保护密钥因子是OBUUID两遍
	callbackMessage("替换OBU系统主控密钥");

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	callbackMessage("从前置获取密钥");
	ret = m_pCmd->cmd_1042(bVer,szOBUID,
							0x01,
							sRnd,
							(BYTE *)"\x84\xD4\x01\x00\x24",
							3,(BYTE *)"\x00\x00\x00",
							szOBUID,
							szEncKey,
							szMAC);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	callbackMessage("装载密钥");

	memset(strCmd,0x00,128);
	memcpy(strCmd,"84D4010024",10);
	CMisc::ByteToString(szEncKey,32,strCmd+10);
	CMisc::ByteToString(szMAC,4,strCmd+74);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;安装MF下的其他密钥
	callbackMessage("安装MF下的其他密钥");

	//	装载维护密钥：	84D401FF	1101FE  010100（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n安装密钥：%s",pkey->strDesc);
		ret = obu_load_key(bVer,szAPPID,szOBUID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret)
		{
			sprintf(strCmd,"失败，返回:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}


	//	进入DF01目录：	00A4000002 DF01
	callbackMessage("进入DF01目录");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A4000002DF01",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//应用主控密钥：	84D401001C 	2002FE	000000（密钥头）	分散因子是OBUUID重复两遍，保护密钥因子是OBUUID两遍
	callbackMessage("安装应用主控密钥");

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret) return ret;

	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	callbackMessage("从前置获取应用主控密钥");
	ret = m_pCmd->cmd_1042(bVer,szOBUID,
							0x03,
							sRnd,
							(BYTE *)"\x84\xD4\x01\x00\x24",
							3,(BYTE *)"\x00\x00\x00",
							szOBUID,
							szEncKey,
							szMAC);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	memset(strCmd,0x00,128);
	memcpy(strCmd,"84D4010024",10);
	CMisc::ByteToString(szEncKey,32,strCmd+10);
	CMisc::ByteToString(szMAC,4,strCmd+74);

	callbackMessage("装载应用主控密钥");

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	/*
	应用维护密钥：	84D401FF1C	2101FE	010100（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
	装载认证密钥：	84D401FF1C	2201FE	010200（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
	装载加密密钥1：	84D401FF1C	2301FE	010300（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
	装载加密密钥2：	84D401FF1C	2302FE	010301（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
	装载加密密钥3：	84D401FF1C	2303FE	010302（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
	*/
	callbackMessage("装载应用下的密钥");
	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n安装密钥：%s",pkey->strDesc);
		ret = obu_load_key(bVer,szAPPID,szOBUID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret)
		{
			sprintf(strCmd,"失败，返回:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}


	//回到主目录：	00A4000002 3F00
	//更新	系统信息文件：	04D681001F	C4FECFC4C4FECFC400FF strID 200811282013112800
	callbackMessage("更新	系统信息文件");

	ret = update_mf_elf01(bVer,szAPPID,elf01_mf);
	if(ret)
	{
		PRINTK("\n更新 系统信息文件 失败：%04X",ret);
		return ret;
	}

	//再次进入DF01:	00A4000002 DF01
	//更新 车辆信息文件 	04D68100 Lc' 车辆信息文件
	callbackMessage("更新 车辆信息文件");

	ret = update_adf_elf01(bVer,szAPPID,elf01_adf);
	if(ret)
	{
		PRINTK("\n更新 车辆信息文件 失败：%04X",ret);
		return ret;
	}

	return 0;
}


//	卡片清除
/*-------------------------------------------------------------------------
Function:		COBUCard.clear
Created:		2018-07-27 14:07:47
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCard::clear()
{
	return 0;
}


/*-------------------------------------------------------------------------
Function:		COBUCard.read_obu
Created:		2018-07-27 14:07:50
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCard::read_obu(BYTE *elf01_mk,BYTE *elf01_adf01)
{
	int ret;
	char strresp[256];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 读取EF01文件：	系统信息文件
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00B081002B",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf01_mk);

	//	2. 选择DF01 ADF
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A4000002DF01",strresp);
	if(ret) return ret;

	//	3. 读取0015文件：	卡发行基本数据文件
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00B400000A00000000000000004F40",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf01_adf01);

	return 0;
}


/*-------------------------------------------------------------------------
Function:		COBUCard.update_mf_elf01
Created:		2018-07-27 14:07:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCard::update_mf_elf01(BYTE bVer,BYTE *szAPPID,BYTE *szELF01)
{
	int ret,i;
	char strresp[256],strCmd[512];
	BYTE szMAC[4],szRnd[16],szCmd[256];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 取随机数
	memset(szRnd,0x00,16);
	memset(strresp,0x00,64);
	ret = preader->RunCmd("0084000004",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,szRnd);

	//	2. 计算MAC
	memset(szMAC,0x00,4);
	memset(szCmd,0x00,256);
	memcpy(szCmd,"\x04\xD6\x81\x00\x67",5);
	memcpy(szCmd+5,szELF01,99);

	ret = m_pCmd->cmd_1034(bVer,szAPPID,szRnd,0x01,104,szCmd,szMAC);
	if(ret) return ret;

	//	3. 写入EF01
	strcpy(strCmd,"04D6810067");
	for(i=0;i<99;i++) sprintf(strCmd+10+2*i,"%02X",szELF01[i]);
	for(i=0;i<4;i++) sprintf(strCmd+208+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,256);
	ret = preader->RunCmd(strCmd,strresp);
	if(ret) return ret;


	return 0;
}



/*-------------------------------------------------------------------------
Function:		COBUCard.update_adf_elf01
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCard::update_adf_elf01(BYTE bVer,BYTE *szAPPID,BYTE *szELF01)
{
	int ret,i;
	char strresp[256],strCmd[512];
	BYTE szMAC[4],szRnd[16],szCmd[256];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A4000002DF01",strresp);
	if(ret) return ret;

	//	1. 取随机数
	memset(szRnd,0x00,16);
	memset(strresp,0x00,64);
	ret = preader->RunCmd("0084000004",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,szRnd);

	//	2. 计算MAC
	memset(szMAC,0x00,4);
	memset(szCmd,0x00,256);
	memcpy(szCmd,"\x04\xD6\x81\x00\x53",5);
	memcpy(szCmd+5,szELF01,79);

	ret = m_pCmd->cmd_1034(bVer,szAPPID,szRnd,0x02,84,szCmd,szMAC);
	if(ret) return ret;

	//	3. 写入EF01
	strcpy(strCmd,"04D6810053");
	for(i=0;i<79;i++) sprintf(strCmd+10+2*i,"%02X",szELF01[i]);
	for(i=0;i<4;i++) sprintf(strCmd+168+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,256);
	ret = preader->RunCmd(strCmd,strresp);
	if(ret) return ret;


	return 0;
}



int COBUCard::update_load_flag(BYTE bVer,BYTE *szAPPID,BYTE bFlag)
{
	int ret,i;
	char strresp[256],strCmd[512];
	BYTE szMAC[4],szRnd[16],szCmd[256];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 取随机数
	memset(szRnd,0x00,16);
	memset(strresp,0x00,64);
	ret = preader->RunCmd("0084000004",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,szRnd);

	//	2. 计算MAC
	memset(szMAC,0x00,4);
	memset(szCmd,0x00,256);
	memcpy(szCmd,"\x04\xD6\x81\x1A\x05",5);
	szCmd[5] = bFlag;

	ret = m_pCmd->cmd_1038(bVer,szAPPID,szRnd,6,szCmd,szMAC);
	if(ret) return ret;

	//	3. 更新拆卸标志
	strcpy(strCmd,"04D6811A05");
	sprintf(strCmd+10,"%02X",bFlag);
	for(i=0;i<4;i++) sprintf(strCmd+12+2*i,"%02X",szMAC[i]);

	memset(strresp,0x00,256);
	ret = preader->RunCmd(strCmd,strresp);
	if(ret) return ret;


	return 0;
}