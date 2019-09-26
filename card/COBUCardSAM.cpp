
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

	//	1. ȡ�����
	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret) return ret;
	memset(strRand,0x00,9);
	memcpy(strRand,strResp,8);

	//	2. 804000010E 1002FE000000 OBUID[4] OBUID[4]����ɢ��Կ��
	memset(strCmd,0x00,128);
	sprintf(strCmd,"804000010E%s%s",strKeyHeader,strDID);
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd(strCmd,strResp);
	if(ret) return ret;
	
	//	3. 8042010014 84D401FF �����[4] 00000000 OBUID[4] OBUID[4]������������
	memset(strCmd,0x00,128);
	sprintf(strCmd,"8042010014%s%s00000000%s",sAPDU,strRand,strPID);
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd(strCmd,strResp);
	if(ret) return ret;


	//	3. װ����Կ
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
				���ڲ��﷢�е�OBU�����⣬��������һ��Ԥ��������ʵ�֡�
-------------------------------------------------------------------------*/
int COBUCardSAM::preInit(WORD wDFID,BYTE *elf01_mf)
{
	PRINTK("\n��֧�� OBUԤ�ȳ�ʼ��");
	return -1;
}

//	��ʼ��
/*-------------------------------------------------------------------------
Function:		COBUCardSAM.init
Created:		2018-07-27 14:07:43
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				elf01_mf	[in]		ϵͳ��Ϣ�ļ���27�ֽ�
				elf01_adf	[in]		������Ϣ�ļ�, 59�ֽڡ�
-------------------------------------------------------------------------*/
int COBUCardSAM::init(BYTE *elf01_mf,BYTE *elf01_adf)
{
	int		ret;
	OBUKEY *pkey;
	BYTE	i,bVer,szAPPID[8];
	
	char	strCmd[512],strResp[128];
	BYTE	bLen;
	char	strOBUID[17],strAPPID[17];

	//	MF�µ���Կ
	OBUKEY arr_mf_keys[] = 
	{
		{"84D40100","1002FE000000",strOBUID,strOBUID,"3DES ϵͳ������Կ CCK_MF"},
		{"84D401FF","1101FE010100",strOBUID,strAPPID,"3DES ϵͳά����Կ CMK_MF"},

		{0,0,NULL,NULL}
	};

	//	ADF1�µ���Կ
	OBUKEY arr_adf_keys[] = 
	{
		//{"84D40100","2002FE000000",strOBUID,strOBUID,"3DES Ӧ��������Կ ACK_ADF01"},
		//{"84D401FF","2101FE010100",strOBUID,strAPPID,"3DES Ӧ��ά����Կ AMK_ADF01"},
		
		{"84D401FF","2201FE010200",strOBUID,strAPPID,"3DES ��֤����Կ IK1_MF"},

		{"84D401FF","2301FE010300",strOBUID,strAPPID,"3DES ������Կ RK1_ADF01"},
		{"84D401FF","2302FE010301",strOBUID,strAPPID,"3DES ������Կ2 RK2_ADF01"},
		{"84D401FF","2303FE010302",strOBUID,strAPPID,"3DES ������Կ3 RK3_ADF01"},
		
		{"84D401FF","2401FE000100",strOBUID,strAPPID,"3DES �ⲿ��֤��Կ UK1_ADF01"},

		{0,0,NULL,NULL}
	};

	//	�汾�ź�Ӧ�����к�
	bVer = elf01_mf[9];
	memcpy(szAPPID,elf01_mf+10,8);

	memset(strAPPID,0x00,17);
	for(i=0;i<8;i++) sprintf(strAPPID+2*i,"%02X",szAPPID[i]);
	
	//	PSAM����λ
	bLen = 0x00;
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_Atr(m_bSAMNode,bLen,strResp);
	if(ret)
	{
		PRINTK("\nSAM����λʧ�ܣ�%d",ret);
		return ret;
	}
	PRINTK("\nPSAM ATR:%s",strResp);

	//	PSAM��ѡ��EF01
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00A4000002EF10",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	PSAM����ȡEF10
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00B0000012",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	PSAM��Verify PIN;
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00200001083132333435363738",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	PSAM��ѡ��A001
	memset(strResp,0x00,128);
	ret = m_pReader->PSAM_RunCmd("00A4000002A001",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	��ȡOBU�ţ�		80F6000304 		����4�ֽڣ�����OBUUID
	callbackMessage("��ȡOBU��");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80F6000304",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(strOBUID,0x00,17);
	memcpy(strOBUID,strResp,8);
	memcpy(strOBUID+8,strResp,8);

	//	ѡ��3F00:	00A40000023F00
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


	//;	��װMF�µ���Կ
	callbackMessage("��װMF�µ���Կ");

	PRINTK("\n�����滻ϵͳά����Կ:");
	ret = obu_load_key("84D401FF","1101FE010100",strOBUID,strAPPID);
	if(ret)
	{
		//	װ��ά����Կ��	84D401FF	1101FE  010100����Կͷ��	��ɢ�����Ǻ�ͬ�ţ�������Կ������OBUUID����
		for(pkey=arr_mf_keys;pkey->strKeyHeader;pkey++)
		{
			PRINTK("\n��װ��Կ��%s",pkey->strDesc);
			ret = obu_load_key(pkey->strAPDU,pkey->strKeyHeader,pkey->strPID,pkey->strDID);
			if(ret)
			{
				sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
				callbackMessage(strCmd);
				return ret;
			}
		}
	}

	//	����DF01Ŀ¼��	00A4000002 DF01
	callbackMessage("����DF01Ŀ¼");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A4000002DF01",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//{"84D401FF","2101FE010100",strOBUID,strAPPID,"3DES Ӧ��ά����Կ AMK_ADF01"},
	PRINTK("\n�����滻Ӧ��ά����Կ:");
	ret = obu_load_key("84D401FF","2101FE010100",strOBUID,strAPPID);
	if(ret)
	{
		PRINTK("\nװ��Ӧ��������Կ��");
		ret = obu_load_key("84D40100","2002FE000000",strOBUID,strOBUID);
		if(ret) 
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}

		//	װ��Ӧ��ά����Կ
		PRINTK("\nװ��Ӧ��ά����Կ:");
		ret = obu_load_key("84D401FF","2101FE010100",strOBUID,strAPPID);
		if(ret) 
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	callbackMessage("װ��Ӧ���µ���Կ");
	for(pkey=arr_adf_keys;pkey->strKeyHeader;pkey++)
	{
		PRINTK("\n��װ��Կ��%s",pkey->strDesc);
		ret = obu_load_key(pkey->strAPDU,pkey->strKeyHeader,pkey->strPID,pkey->strDID);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}
	//�ص���Ŀ¼��	00A4000002 3F00
	//����	ϵͳ��Ϣ�ļ���	04D681001F	C4FECFC4C4FECFC400FF strID 200811282013112800
	callbackMessage("���� ϵͳ��Ϣ�ļ�");

	memset(strCmd,0x00,512);
	strcpy(strCmd,"04D681001F");
	for(i=0;i<27;i++) sprintf(strCmd+10+2*i,"%02X",elf01_mf[i]);
	ret = update_mf_elf01("1101FE010100",strAPPID,strCmd);
	if(ret)
	{
		PRINTK("\n���� ϵͳ��Ϣ�ļ� ʧ�ܣ�%04X",ret);
		return ret;
	}

	//�ٴν���DF01:	00A4000002 DF01
	//���� ������Ϣ�ļ� 	04D68100 Lc' ������Ϣ�ļ�
	if(NULL==elf01_adf) return 0;
	
	callbackMessage("���� ������Ϣ�ļ�");
	memset(strCmd,0x00,27);
	strcpy(strCmd,"04D681003F");
	for(i=0;i<59;i++) sprintf(strCmd+10+2*i,"%02X",elf01_adf[i]);

	ret = update_adf_elf01("2101FE010100",strAPPID,strCmd);
	if(ret)
	{
		PRINTK("\n���� ������Ϣ�ļ� ʧ�ܣ�%04X",ret);
		return ret;
	}

	return 0;
}


//	��Ƭ���
/*-------------------------------------------------------------------------
Function:		COBUCardSAM.clear
Created:		2018-07-27 14:07:47
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int COBUCardSAM::clear()
{
	PRINTK("\n��֧�� OBU���");
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

	//	0. ѡ��3F00Ŀ¼
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. ��ȡEF01�ļ���ϵͳ��Ϣ�ļ�
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
				����OBU�� ϵͳ��Ϣ�ļ�������+MAC
-------------------------------------------------------------------------*/
int COBUCardSAM::update_mf_elf01(char *strKeyHeader,char *strDID,char *strAPDU)
{
	int ret;
	char strresp[256],strCmd[512];
	char strRand[9];

	//	0. ѡ��3F00Ŀ¼
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. ȡ�����
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	memset(strRand,0x00,9);
	memcpy(strRand,strresp,8);

	//804000010E 1101FE010100 + Ӧ�����к�[8]
	memset(strCmd,0x00,512);
	sprintf(strCmd,"804000010E%s%s",strKeyHeader,strDID);
	memset(strresp,0x00,64);
	ret = m_pReader->PSAM_RunCmd(strCmd,strresp);
	if(ret) return ret;

	//80F8050030 �����[4] 00000000 04D681001F + ϵͳ��Ϣ�ļ�[] + 8000000000000000
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
				����OBU�� ������Ϣ�ļ�������+MAC
-------------------------------------------------------------------------*/
int COBUCardSAM::update_adf_elf01(char *strKeyHeader,char *strDID,char *strAPDU)
{
	int ret;
	char strresp[256],strCmd[512];
	char strRand[9];

	//	0. ѡ��3F00Ŀ¼
	memset(strresp,0x00,256);
	ret = m_pReader->RunCmd("00A4000002DF01",strresp);
	if(ret) return ret;

	//	1. ȡ�����
	memset(strresp,0x00,64);
	ret = m_pReader->RunCmd("0084000004",strresp);
	if(ret) return ret;

	memset(strRand,0x00,9);
	memcpy(strRand,strresp,8);

	//804000010E 1101FE010100 + Ӧ�����к�[8]
	memset(strCmd,0x00,512);
	sprintf(strCmd,"804000010E%s%s",strKeyHeader,strDID);
	memset(strresp,0x00,64);
	ret = m_pReader->PSAM_RunCmd(strCmd,strresp);
	if(ret) return ret;

	//80F8050050 �����[4] 00000000 04D681003F + ������Ϣ�ļ�[59] + 8000000000000000
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
				�޸�OBU�Ĳ�ж��־
-------------------------------------------------------------------------*/
int COBUCardSAM::update_load_flag(BYTE bVer,BYTE *szAPPID,BYTE bFlag)
{
	PRINTK("\n��֧�� �޸�OBU�Ĳ�ж��־");
	return -1;
}

/*-------------------------------------------------------------------------
Function:		COBUCardSAM.getOBUUID
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				��ȡOBU оƬ���к�
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
		PRINTK("ʧ�ܣ�����:%04X",ret);
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
				���� ������Ϣ�ļ�
-------------------------------------------------------------------------*/
int COBUCardSAM::read_vechile_file(BYTE bNode,BYTE bVer,BYTE *szPlainFile)
{

	PRINTK("\n��֧�� ������Ϣ�ļ�����");
	return -1;
}


/*-------------------------------------------------------------------------
Function:		COBUCardSAM.unlockapp
Created:		2018-07-27 14:07:59
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				���� OBU��DF01Ӧ��
-------------------------------------------------------------------------*/
int COBUCardSAM::unlockapp(BYTE bVer,BYTE *szAPPID)
{

	PRINTK("\n��֧�ֽ���");
	return -1;
}


void COBUCardSAM::setSAMNode(BYTE bNode)
{
	m_bSAMNode = bNode;
}