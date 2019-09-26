
/*-------------------------------------------------------------------------
Function:		COBUCardSAM.COBUCardSAM
Created:		2018-07-27 14:07:19
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
COBUCardSAM::COBUCardSAM()
{
	m_pCmd=NULL;
	m_pReader=NULL;

	m_bSAMNode = 0x01;
}

/*-------------------------------------------------------------------------
Function:		COBUCardSAM.COBUCardSAM
Created:		2018-07-27 14:07:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
COBUCardSAM::COBUCardSAM(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd=pcmd;
	m_pReader=preader;
}

/*-------------------------------------------------------------------------
Function:		COBUCardSAM.~COBUCardSAM
Created:		2018-07-27 14:07:37
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
COBUCardSAM::~COBUCardSAM()
{
}



/*-------------------------------------------------------------------------
Function:		COBUCardSAM.obu_load_key
Created:		2018-07-28 11:34:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCardSAM::obu_load_key(char *sAPDU,char *strKeyHeader,char *strPID,char *strDID)
{
	int		ret;
	char	strRand[9];
	char	strCmd[128],strResp[128];

	//	1. 取随机数
	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret) return ret;
	memset(strRand,0x00,9);
	memcpy(strRand,strResp,8);

	//	2. 804000010E 1002FE000000 OBUID[4] OBUID[4]（分散密钥）
	memset(strCmd,0x00,128);
	sprintf(strCmd,"804000010E%s%s",strKeyHeader,strDID);
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd(strCmd,strResp);
	if(ret) return ret;
	
	//	3. 8042010014 84D401FF 随机数[4] 00000000 OBUID[4] OBUID[4]（保护导出）
	memset(strCmd,0x00,128);
	sprintf(strCmd,"8042010014%s%s00000000%s",sAPDU,strRand,strPID);
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd(strCmd,strResp);
	if(ret) return ret;


	//	3. 装载密钥
	memset(strCmd,0x00,128);
	sprintf(strCmd,"%s1C%s",sAPDU,strResp);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	return ret;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.preInit
Created:		2018-07-28 11:34:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				由于部里发行的OBU有问题，所以做了一个预处理功能来实现。
-------------------------------------------------------------------------*/
int COBUCardSAM::preInit(WORD wDFID,BYTE *elf01_mf)
{
	PRINTK("\n不支持 OBU预先初始化");
	return -1;
}

//	初始化
/*-------------------------------------------------------------------------
Function:		COBUCardSAM.init
Created:		2018-07-27 14:07:43
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				elf01_mf	[in]		系统信息文件，27字节
				elf01_adf	[in]		车辆信息文件, 59字节。
-------------------------------------------------------------------------*/
int COBUCardSAM::init(BYTE *elf01_mf,BYTE *elf01_adf)
{
	int		ret;
	OBUKEY *pkey;
	BYTE	i,bVer,szAPPID[8];
	
	char	strCmd[512],strResp[128];
	BYTE	bLen;
	char	strOBUID[17],strAPPID[17];

	//	MF下的密钥
	OBUKEY arr_mf_keys[] = 
	{
		{"84D40100","1002FE000000",strOBUID,strOBUID,"3DES 系统主控密钥 CCK_MF"},
		{"84D401FF","1101FE010100",strOBUID,strAPPID,"3DES 系统维护密钥 CMK_MF"},

		{0,0,NULL,NULL}
	};

	//	ADF1下的密钥
	OBUKEY arr_adf_keys[] = 
	{
		//{"84D40100","2002FE000000",strOBUID,strOBUID,"3DES 应用主控密钥 ACK_ADF01"},
		//{"84D401FF","2101FE010100",strOBUID,strAPPID,"3DES 应用维护密钥 AMK_ADF01"},
		
		{"84D401FF","2201FE010200",strOBUID,strAPPID,"3DES 认证主密钥 IK1_MF"},

		{"84D401FF","2301FE010300",strOBUID,strAPPID,"3DES 加密密钥 RK1_ADF01"},
		{"84D401FF","2302FE010301",strOBUID,strAPPID,"3DES 加密密钥2 RK2_ADF01"},
		{"84D401FF","2303FE010302",strOBUID,strAPPID,"3DES 加密密钥3 RK3_ADF01"},
		
		{"84D401FF","2401FE000100",strOBUID,strAPPID,"3DES 外部认证密钥 UK1_ADF01"},

		{0,0,NULL,NULL}
	};

	//	版本号和应用序列号
	bVer = elf01_mf[9];
	memcpy(szAPPID,elf01_mf+10,8);

	memset(strAPPID,0x00,17);
	for(i=0;i<8;i++) sprintf(strAPPID+2*i,"%02X",szAPPID[i]);
	
	//	PSAM：复位
	bLen = 0x00;
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_Atr(m_bSAMNode,bLen,strResp);
	if(ret)
	{
		PRINTK("\nSAM卡复位失败：%d",ret);
		return ret;
	}
	PRINTK("\nPSAM ATR:%s",strResp);

	//	PSAM：选择EF01
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00A4000002EF10",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	PSAM：读取EF10
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00B0000012",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	PSAM：Verify PIN;
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00200001083132333435363738",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	PSAM：选择A001
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00A4000002A001",strResp);
	if(ret)
	{
		sprintf(strCmd,"失败，返回:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

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
	memset(strOBUID,0x00,17);
	memcpy(strOBUID,strResp,8);
	memcpy(strOBUID+8,strResp,8);

	//	选择3F00:	00A40000023F00
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


	//;	安装MF下的密钥
	callbackMessage("安装MF下的密钥");

	PRINTK("\n尝试替换系统维护密钥:");
	ret = obu_load_key("84D401FF","1101FE010100",strOBUID,strAPPID);
	if(ret)
	{
		//	装载维护密钥：	84D401FF	1101FE  010100（密钥头）	分散因子是合同号，保护密钥因子是OBUUID两遍
		for(pkey=arr_mf_keys;pkey->strKeyHeader;pkey++)
		{
			PRINTK("\n安装密钥：%s",pkey->strDesc);
			ret = obu_load_key(pkey->strAPDU,pkey->strKeyHeader,pkey->strPID,pkey->strDID);
			if(ret)
			{
				sprintf(strCmd,"失败，返回:%04X",ret);
				callbackMessage(strCmd);
				return ret;
			}
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

	//{"84D401FF","2101FE010100",strOBUID,strAPPID,"3DES 应用维护密钥 AMK_ADF01"},
	PRINTK("\n尝试替换应用维护密钥:");
	ret = obu_load_key("84D401FF","2101FE010100",strOBUID,strAPPID);
	if(ret)
	{
		PRINTK("\n装载应用主控密钥：");
		ret = obu_load_key("84D40100","2002FE000000",strOBUID,strOBUID);
		if(ret) 
		{
			sprintf(strCmd,"失败，返回:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}

		//	装载应用维护密钥
		PRINTK("\n装载应用维护密钥:");
		ret = obu_load_key("84D401FF","2101FE010100",strOBUID,strAPPID);
		if(ret) 
		{
			sprintf(strCmd,"失败，返回:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	callbackMessage("装载应用下的密钥");
	for(pkey=arr_adf_keys;pkey->strKeyHeader;pkey++)
	{
		PRINTK("\n安装密钥：%s",pkey->strDesc);
		ret = obu_load_key(pkey->strAPDU,pkey->strKeyHeader,pkey->strPID,pkey->strDID);
		if(ret)
		{
			sprintf(strCmd,"失败，返回:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}
	//回到主目录：	00A4000002 3F00
	//更新	系统信息文件：	04D681001F	C4FECFC4C4FECFC400FF strID 200811282013112800
	callbackMessage("更新 系统信息文件");

	memset(strCmd,0x00,512);
	strcpy(strCmd,"04D681001F");
	for(i=0;i<27;i++) sprintf(strCmd+10+2*i,"%02X",elf01_mf[i]);
	ret = update_mf_elf01("1101FE010100",strAPPID,strCmd);
	if(ret)
	{
		PRINTK("\n更新 系统信息文件 失败：%04X",ret);
		return ret;
	}

	//再次进入DF01:	00A4000002 DF01
	//更新 车辆信息文件 	04D68100 Lc' 车辆信息文件
	if(NULL==elf01_adf) return 0;
	
	callbackMessage("更新 车辆信息文件");
	memset(strCmd,0x00,27);
	strcpy(strCmd,"04D681003F");
	for(i=0;i<59;i++) sprintf(strCmd+10+2*i,"%02X",elf01_adf[i]);

	ret = update_adf_elf01("2101FE010100",strAPPID,strCmd);
	if(ret)
	{
		PRINTK("\n更新 车辆信息文件 失败：%04X",ret);
		return ret;
	}

	return 0;
}


//	卡片清除
/*-------------------------------------------------------------------------
Function:		COBUCardSAM.clear
Created:		2018-07-27 14:07:47
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCardSAM::clear()
{
	PRINTK("\n不支持 OBU清除");
	return -1;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.read_obu
Created:		2018-07-27 14:07:50
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCardSAM::read_obu(BYTE *elf01_mk)
{
	int ret;
	char strresp[256];

	if(!validation()) return -1;

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 读取EF01文件：系统信息文件
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00B081002B",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf01_mk);

	return 0;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.update_mf_elf01
Created:		2018-07-27 14:07:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				更新OBU的 系统信息文件，明文+MAC
-------------------------------------------------------------------------*/
int COBUCardSAM::update_mf_elf01(char *strKeyHeader,char *strDID,char *strAPDU)
{
	int ret;
	char strresp[256],strCmd[512];
	char strRand[9];

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. 取随机数
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	memset(strRand,0x00,9);
	memcpy(strRand,strresp,8);

	//804000010E 1101FE010100 + 应用序列号[8]
	memset(strCmd,0x00,512);
	sprintf(strCmd,"804000010E%s%s",strKeyHeader,strDID);
	memset(strresp,0x00,64);
	ret = m_pReader->PSAM_RunCmd(strCmd,strresp);
	if(ret) return ret;

	//80F8050030 随机数[4] 00000000 04D681001F + 系统信息文件[] + 8000000000000000
	memset(strCmd,0x00,512);
	sprintf(strCmd,"80F8050030%s00000000%s8000000000000000",strRand,strAPDU);
	memset(strresp,0x00,64);
	ret = m_pReader->PSAM_RunCmd(strCmd,strresp);
	if(ret) return ret;
	
	memset(strRand,0x00,9);
	memcpy(strRand,strresp,8);

	sprintf(strCmd,"%s%s",strAPDU,strRand);
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}



/*-------------------------------------------------------------------------
Function:		COBUCardSAM.update_adf_elf01
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				更新OBU的 车辆信息文件，明文+MAC
-------------------------------------------------------------------------*/
int COBUCardSAM::update_adf_elf01(char *strKeyHeader,char *strDID,char *strAPDU)
{
	int ret;
	char strresp[256],strCmd[512];
	char strRand[9];

	//	0. 选择3F00目录
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A4000002DF01",strresp);
	if(ret) return ret;

	//	1. 取随机数
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	memset(strRand,0x00,9);
	memcpy(strRand,strresp,8);

	//804000010E 1101FE010100 + 应用序列号[8]
	memset(strCmd,0x00,512);
	sprintf(strCmd,"804000010E%s%s",strKeyHeader,strDID);
	memset(strresp,0x00,64);
	ret = m_pReader->PSAM_RunCmd(strCmd,strresp);
	if(ret) return ret;

	//80F8050050 随机数[4] 00000000 04D681003F + 车辆信息文件[59] + 8000000000000000
	memset(strCmd,0x00,512);
	sprintf(strCmd,"80F8050050%s00000000%s8000000000000000",strRand,strAPDU);
	memset(strresp,0x00,64);
	ret = m_pReader->PSAM_RunCmd(strCmd,strresp);
	if(ret) return ret;
	
	memset(strRand,0x00,9);
	memcpy(strRand,strresp,8);

	sprintf(strCmd,"%s%s",strAPDU,strRand);
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd(strCmd,strresp);
	if(ret) return ret;

	return 0;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.update_load_flag
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				修改OBU的拆卸标志
-------------------------------------------------------------------------*/
int COBUCardSAM::update_load_flag(BYTE bVer,BYTE *szAPPID,BYTE bFlag)
{
	PRINTK("\n不支持 修改OBU的拆卸标志");
	return -1;
}

/*-------------------------------------------------------------------------
Function:		COBUCardSAM.getOBUUID
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				读取OBU 芯片序列号
-------------------------------------------------------------------------*/
int COBUCardSAM::getOBUUID(BYTE *szUID)
{
	int		ret;

	char	strResp[128];

	if(!validation()) return -1;
	
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F6000304",strResp);
	if(ret)
	{
		PRINTK("失败，返回:%04X",ret);
		return ret;
	}

	CMisc::StringToByte(strResp,szUID);
	return 0;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.read_vechile_file
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				解密 车辆信息文件
-------------------------------------------------------------------------*/
int COBUCardSAM::read_vechile_file(BYTE bNode,BYTE bVer,BYTE *szPlainFile)
{

	PRINTK("\n不支持 车辆信息文件解密");
	return -1;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.unlockapp
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				解密 OBU的DF01应用
-------------------------------------------------------------------------*/
int COBUCardSAM::unlockapp(BYTE bVer,BYTE *szAPPID)
{

	PRINTK("\n不支持解锁");
	return -1;
}


void COBUCardSAM::setSAMNode(BYTE bNode)
{
	m_bSAMNode = bNode;
}