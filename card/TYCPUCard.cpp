/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/24 16:12:34

    Reversion:
        
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
Function:		CTYCPUCard.CTYCPUCard
Created:		2018-07-24 16:12:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTYCPUCard::CTYCPUCard()
{
	m_pCmd=NULL;
	m_pReader=NULL;
}

/*-------------------------------------------------------------------------
Function:		CTYCPUCard.CTYCPUCard
Created:		2018-07-24 16:12:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTYCPUCard::CTYCPUCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;
}

/*-------------------------------------------------------------------------
Function:		CTYCPUCard.~CTYCPUCard
Created:		2018-07-24 16:12:46
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CTYCPUCard::~CTYCPUCard()
{

}

/*-------------------------------------------------------------------------
Function:		CTYCPUCard.ty_load_key
Created:		2018-07-24 16:12:49
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTYCPUCard::ty_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader)
{
	int		ret;
	BYTE	sRnd[8];
	char	strCmd[128],strResp[32];
	BYTE	szEncKey[32],szMAC[4];

	//	1. ȡ�����
	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret) return ret;

	memset(sRnd,0x00,8);
	CMisc::StringToByte(strResp,sRnd);

	//	2. ��ȡ��Կ���ĺ�MAC
	ret = m_pCmd->cmd_1040(bVer,szAPPID,
							bKeyNo,
							sRnd,
							(BYTE *)"\x84\xD4\x00\x00\x24",
							bKeyHeaderLen,szKeyHeader,
							szDID,
							szEncKey,
							szMAC);
	if(ret) return ret;

	//	3. װ����Կ
	memset(strCmd,0x00,128);
	memcpy(strCmd,"84D4000024",10);
	CMisc::ByteToString(szEncKey,32,strCmd+10);
	CMisc::ByteToString(szMAC,4,strCmd+74);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	
	return ret;
}



//	��ʼ��
/*-------------------------------------------------------------------------
Function:		CTYCPUCard.init
Created:		2018-07-24 16:12:53
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTYCPUCard::init(BYTE *elf15)
{
	int		i,ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	szEncKey[32],szMAC[4];
	BYTE	ENCODER_INIT_KEY[16],szBuf[64],szSubKey[16];
	BYTE	CARD_INIT_KEY[16];

	//memcpy(INIT_KEY,"\xB5\xDD\x74\x72\x1B\x1E\x81\xD0\x08\x80\x91\x97\x45\xE2\x9D\xA2",16);

	//	���ܻ���ȱʡ������Կ
	memset(strCmd,0x00,512);
	GetPrivateProfileString("ENCODER","INITKEY","",strCmd,33,".\\key.ini");
	if(strlen(strCmd)==0)
	{
		strcpy(strCmd,"B5DD74721B1E81D00880919745E29DA2");
		WritePrivateProfileString("ENCODER","INITKEY",strCmd,".\\key.ini");
	}
	CMisc::StringToByte(strCmd,ENCODER_INIT_KEY);
	//PRINTK("\nENCODER INIT KEY: %s",strCmd);

	//	�հ��û���ȱʡ������Կ
	memset(strCmd,0x00,512);
	GetPrivateProfileString("CARD","INITKEY","",strCmd,33,".\\key.ini");
	if(strlen(strCmd)==0)
	{
		strcpy(strCmd,"B5DD74721B1E81D00880919745E29DA2");
		WritePrivateProfileString("CARD","INITKEY",strCmd,".\\key.ini");
	}
	CMisc::StringToByte(strCmd,CARD_INIT_KEY);
	//PRINTK("\nCARD INIT KEY: %s",strCmd);

	//	MF�µ���Կ
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,8,(BYTE *)"\x41\x01\x04\x05\x0F\x0F\x0F\x33","SM4 ��Ƭά����Կ DAMK_DF01",NULL},

		{0,0,NULL,NULL}
	};

	CARDKEY arr_adf_keys[] = 
	{
		{0x07,8,(BYTE *)"\x40\x01\x04\x08\x0F\x0F\x0F\x33","SM4 Ӧ��������Կ MK_DF01",NULL},
		{0x08,8,(BYTE *)"\x41\x01\x04\x05\x0F\x0F\x0F\x33","SM4 Ӧ��ά����Կ DAMK_DF01",NULL},
		
		{0x03,8,(BYTE *)"\x01\x01\x00\x08\x0F\x02\x0F\x33","3DES �ⲿ��֤����Կ1 UK1_DF01",NULL},
		{0x10,8,(BYTE *)"\x00\x01\x00\x09\x0F\x00\x0F\xFF","3DES �ڲ���֤����Կ1 IK1_DF01",NULL},
		{0x01,8,(BYTE *)"\x01\x01\x00\x00\x0F\x00\x0F\xFF","3DES ��������Կ1 DPK1",NULL},
		{0x02,8,(BYTE *)"\x02\x01\x00\x00\x0F\x00\x0F\xFF","3DES ��������Կ2 DPK2",NULL},
		{0x04,8,(BYTE *)"\x00\x01\x00\x02\x0F\x00\x0F\xFF","3DES TAC����Կ1 DTK1",NULL},
		/*	PIN��Կ*/
		{0x0B,8,(BYTE *)"\x00\x01\x00\x06\x0F\x00\x0F\xFF","3DES Ӧ��PIN��������Կ1 DPUK1_DF01",NULL},
		{0x0C,8,(BYTE *)"\x01\x01\x00\x07\x0F\x00\x0F\xFF","3DES Ӧ��PIN��װ����Կ1 DPRK1_DF01",NULL},
			
		{0x21,8,(BYTE *)"\x41\x41\x04\x08\x0F\x02\x0F\x33","SM4 �ⲿ��֤����Կ2 UK2_DF01",NULL},
		{0x22,8,(BYTE *)"\x40\x41\x04\x09\x0F\x00\x0F\xFF","SM4 �ڲ���֤����Կ2 IK2_DF01",NULL},
		{0x23,8,(BYTE *)"\x41\x41\x04\x00\x0F\x00\x0F\xFF","SM4 ��������Կ3 DPK3",NULL},
		{0x24,8,(BYTE *)"\x42\x41\x04\x00\x0F\x00\x0F\xFF","SM4 ��������Կ4 DPK4",NULL},
		{0x09,8,(BYTE *)"\x41\x41\x04\x01\x0F\x00\x0F\xFF","SM4 Ȧ������Կ3 DLK3",NULL},
		{0x0A,8,(BYTE *)"\x42\x41\x04\x01\x0F\x00\x0F\xFF","SM4 Ȧ������Կ4 DLK4",NULL},
		{0x27,8,(BYTE *)"\x40\x41\x04\x02\x0F\x00\x0F\xFF","SM4 TAC����Կ2 DTK2",NULL},
		{0x28,8,(BYTE *)"\x40\x41\x04\x06\x0F\x00\x0F\xFF","SM4 Ӧ��PIN��������Կ2 DPUK2_DF01",NULL},
		{0x29,8,(BYTE *)"\x41\x41\x04\x07\x0F\x00\x0F\xFF","SM4 Ӧ��PIN��װ����Կ2 DPRK2_DF01",NULL},

		{0,0,NULL,NULL}
	};

	//	��¼�ļ��е�ʡ�ݱ�ʶ
	BYTE szProviceFlag[34] = {0x11,0x12,0x13,0x14,0x15,
							0x21,0x22,0x23,
							0x31,0x32,0x33,0x34,0x35,0x36,0x37,
							0x41,0x42,0x42,0x44,0x45,0x46,
							0x50,0x51,0x52,0x53,0x54,
							0x61,0x62,0x62,0x64,0x65,
							0x71,0x81,0x82};
	


	//	��Ƭ�汾�ź�Ӧ�����к�
	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	//	; ѡ��1PAY.SYS.DDF01
	callbackMessage("ѡ��1PAY.SYS.DDF01");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00A404000E315041592E5359532E4444463031",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	; ���MF
	callbackMessage("���MF");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("800E000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

 
	//;��������ΪDDF��ROOTDF--3F00�������ļ�Ȩ��Ϊ0F,SFI-01,����:1PAY.SYS.DDF01
	//;0D���0F�޸�ΪFF���Ϳ��Խ�ֹ�ļ����
	callbackMessage("����MF");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E000000D0F0101FF000000000000000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;2F01,KEY�ļ�
	callbackMessage("����MF��KEY�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A2F0105F0001000000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	��װ������Կ�������ĳ�ʼ��Ƭ������Կ�Ǹ���ֵ����Ҫ���⴦��һ�£�
	//			{0x05,8,(BYTE *)"\x40\x01\x04\x08\x0F\x0F\x0F\x33","SM4 ��Ƭ������Կ MK_DF01"},
	callbackMessage("��װ��Ƭ������Կ");

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd("0084000004",strResp);
	if(ret) 
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	memset(sRnd,0x00,16);
	CMisc::StringToByte(strResp,sRnd);

	//	2. ��ȡ��Կ���ĺ�MAC
	ret = m_pCmd->cmd_1040(bVer,szAPPID,
							0x05,
							sRnd,
							(BYTE *)"\x84\xD4\x00\x00\x24",
							8,(BYTE *)"\x40\x01\x04\x08\x0F\x0F\x0F\x33",
							szAPPID,
							szEncKey,
							szMAC);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);	
		return ret;
	}

	//	���ü��ܻ���ʼ���ظ���Կ��ɢ������
	memset(szSubKey,0x00,16);
	sm4_diversify(ENCODER_INIT_KEY,szAPPID,szSubKey);

	memset(szBuf,0x00,64);
	sm4_decode(szSubKey,szEncKey,szBuf);
	sm4_decode(szSubKey,szEncKey+16,szBuf+16);
	
	//	Ȼ���ÿ�Ƭȱʡ������Կ���ܡ�����MAC
	memset(szEncKey,0x00,32);
	sm4_encode(CARD_INIT_KEY,szBuf,szEncKey);
	sm4_encode(CARD_INIT_KEY,szBuf+16,szEncKey+16);
	
	memset(szBuf,0x00,64);
	memcpy(szBuf,(BYTE *)"\x84\xD4\x00\x00\x24",5);
	memcpy(szBuf+5,szEncKey,32);

	memset(szMAC,0x00,4);
	sm4_cal_mac(CARD_INIT_KEY,sRnd,37,szBuf,szMAC);

	//	3. װ�ؿ�Ƭ������Կ
	memset(strCmd,0x00,128);
	memcpy(strCmd,"84D4000024",10);
	CMisc::ByteToString(szEncKey,32,strCmd+10);
	CMisc::ByteToString(szMAC,4,strCmd+74);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);	
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////


	//;��װMF�µ�������Կ
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n��װ��Կ��%s",pkey->strDesc);
		ret = ty_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}

	//;---0016���ֿ��˻��������ļ����������ɣ�дDAMK_MF��·������
	callbackMessage("�����ֿ��˻��������ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0016200F0F0037410041",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;---0001��DIRĿ¼�����ļ����������ɣ�дDAMK_MF��·������
	callbackMessage("����DIRĿ¼�����ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0001220F0F0028410000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	//;---1001��DF01�����շ�Ӧ��Ŀ¼
	callbackMessage("����DF01�����շ�Ӧ��");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E001001010010F00410000A00000000386980701",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;---2F01��KEY�ļ�
	callbackMessage("����ADF��KEY�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A2F01051F001A19000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	��װADF�����е���Կ
	callbackMessage("��װADF��������Կ");

	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\n��װ��Կ��%s",pkey->strDesc);
		ret = ty_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader);
		if(ret) return ret;
	}

	//;	Ӧ��PIN,PIN
	callbackMessage("��װӦ��PIN");
		
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80D400000E0000000B0F010F33313233343536",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	//;0015����Ƭ���л��������ļ����������ɣ�дDAMK1_DF01��·������
	callbackMessage("��װ ��Ƭ���л��������ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0015200F0F0032410000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;	����0015�ļ�
	callbackMessage("���� ��Ƭ���л��������ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);

	memcpy(strCmd,"00D6950032",10);
	for(i=0;i<50;i++) sprintf(strCmd+10+2*i,"%02X",elf15[i]);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;0019�������շѸ������ѹ����ļ����������ɣ�дDAMK1_DF01��·������
	callbackMessage("��װ �����շѸ������ѹ����ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0019220F0F0360410000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	callbackMessage("���� �����շѸ������ѹ����ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00E200002BAA2900FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	for(i=0xb1;i<0xb6;i++)
	{
		sprintf(strCmd,"00E200002B%02X2900FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",i);
		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00E200003FC13D00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00E200003FC23D00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00E2000060D15E00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00E2000060D25E00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;0002������Ǯ���ļ�����COSά����
	callbackMessage("��װ ����Ǯ���ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A00020600000026000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}



	//;0018���ն˽��׼�¼�ļ���������д��COSά����
	callbackMessage("��װ �ն˽��׼�¼�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0018041F100A17000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;0012�������շ���Ϣ�ļ����������ļ����������ɣ�д�ⲿ��֤UK1_DF01��
	callbackMessage("��װ �����շ���Ϣ�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0012000F1F0028010000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;0008����ʶվ�ļ����������ļ����������ɣ�д�ⲿ��֤UK1_DF01��
	callbackMessage("��װ ��ʶվ�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0008000F1F0080010000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;0009�������ļ�6���������ļ����������ɣ�д���ɣ�
	callbackMessage("��װ �����ļ�6");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A0009000F0F0200010000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;001A�������ļ�2���䳤��¼�ļ����������ɣ�дDAMK1_DF01��·������
	callbackMessage("��װ 001A �����ļ�2");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A001A220F0F0450410000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	for(i=0;i<34;i++)
	{
		sprintf(strCmd,"���� 001A�ļ���¼%d",szProviceFlag[i]);
		callbackMessage(strCmd);

		sprintf(strCmd,"00E200001E%02X1C00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",szProviceFlag[i]);
		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}



	//;001B�������ļ�3�������䳤��¼�ļ����������ɣ�д�ⲿ��֤UK1_DF01��
	callbackMessage("��װ 001B �����ļ�3");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A001B020F1F0450010000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	for(i=0;i<34;i++)
	{
		sprintf(strCmd,"���� 001B�ļ���¼%d",szProviceFlag[i]);
		callbackMessage(strCmd);

		sprintf(strCmd,"00E200001E%02X1C00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",szProviceFlag[i]);
		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}


	//;001C�������ļ�4���������ļ����������ɣ�д�ⲿ��֤UK1_DF01��
	callbackMessage("��װ 001C �����ļ�4");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A001C000F1F00FF010000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;001D�������ļ�5���������ļ����������ɣ�д�ⲿ��֤UK1_DF01��
	callbackMessage("��װ 001D �����ļ�5");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A001D000F1F00FF010000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;000E,����ʡ���ļ������ڵ���4X�汾
	callbackMessage("��װ 000E ����ʡ���ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E002000A000E200F0F0046410000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//;	����ADF����
	callbackMessage("����ADF����");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80EA100100",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	//;��������
	callbackMessage("��Ƭ��������");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80EA3F0000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	return 0;
}



//	��Ƭ���
/*-------------------------------------------------------------------------
Function:		CTYCPUCard.clear
Created:		2018-07-24 16:13:00
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CTYCPUCard::clear(BYTE *elf15)
{
	return -1;
}


int CTYCPUCard::preInit(WORD wDFID,BYTE *elf15)
{
	return 0;
}
