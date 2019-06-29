
CWD3DESCard::CWD3DESCard()
{
	m_pCmd = NULL;
	m_pReader = NULL;
}

CWD3DESCard::CWD3DESCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd = pcmd;
	m_pReader = preader;
}

CWD3DESCard::~CWD3DESCard()
{
}


int CWD3DESCard::wd_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *szKey)
{
	int		i,ret;
	char	strCmd[512],strResp[128];
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
		PRINTK("\n1040��ȡKey[%d]ʧ��:%d",bKeyNo,ret);
		return ret;
	}

	des3_decode(szSesKey,szCmd+32,szCmd);
	des3_decode(szSesKey,szCmd+40,szCmd+8);
	des3_decode(szSesKey,szCmd+48,szCmd+16);

	PRINTK("\n");
	for(i=0;i<32;i++) PRINTK("%02X",szCmd[i]);

	memcpy(szKey,szCmd+1+bKeyHeaderLen,16);

	memset(szCmd,0x00,128);
	memcpy(szCmd,"\x80\xD4\x01\x01\x15",5);
	memcpy(szCmd+5,szKeyHeader+1,bKeyHeaderLen);
	memcpy(szCmd+5+bKeyHeaderLen,szKey,16);

	szCmd[3] = szKeyHeader[0];

	memset(strCmd,0x00,512);
	CMisc::ByteToString(szCmd,bKeyHeaderLen+21,strCmd);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"װ����Կʧ�ܣ�����:%04X",ret);
		return ret;
	}

	return 0;
}

//	��ʼ��
int CWD3DESCard::init(BYTE *elf15)
{
	int		i,ret;
	CARDKEY *pkey;
	BYTE	bVer,szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	szCMK[16],szAMK[16];
	BYTE	sBuf[128];
	BYTE	INIT_KEY[16];

	memcpy(INIT_KEY,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);

	//	MF�µ���Կ	KeyHeader�ĵ�һ���ֽ���P2
	CARDKEY arr_mf_keys[] = 
	{
		{0x06,5,(BYTE *)"\x00\xF6\xF0\xF0\xFF\x33","��Ƭά����Կ DAMK_CMK"},

		{0,0,NULL,NULL}
	};


	CARDKEY arr_adf_keys[] = 
	{
		{0x07,5,(BYTE *)"\x00\xF9\xF0\xF0\xAA\x33","Ӧ��������Կ MK_DF01"},
		
		{0x03,5,(BYTE *)"\x01\xF9\xF0\xF0\x02\x33","�ⲿ��֤����Կ UK_DF01"},
		{0x10,5,(BYTE *)"\x00\xF0\xF0\xF0\x01\x00","�ڲ���֤����Կ IK_DF01"},
		{0x01,5,(BYTE *)"\x01\xFE\xF0\xF0\x01\x00","��������Կ1 DPK1"},
		{0x02,5,(BYTE *)"\x02\xFE\xF0\xF0\x01\x00","��������Կ2 DPK2"},
		{0x04,5,(BYTE *)"\x00\xF4\xF0\xF0\x01\x00","TAC����Կ DTK"},

		{0x09,5,(BYTE *)"\x01\xFF\xF0\xF0\x01\x00","Ȧ������Կ1 DLK1"},
		{0x0A,5,(BYTE *)"\x02\xFF\xF0\xF0\x01\x00","Ȧ������Կ2 DLK2"},
		{0x0B,5,(BYTE *)"\x00\xF7\xF0\xF0\xFF\x33","Ӧ��PIN��������Կ DPUK_DF01"},
		{0x0C,5,(BYTE *)"\x00\xF8\xF0\xF0\xFF\x33","Ӧ��PIN��װ����Կ DPRK_DF01"},

		{0,0,NULL,NULL}
	};

	//	Ӧ��ά����Կ
	CARDKEY arr_adf_amk[] = 
	{
		{0x08,5,(BYTE *)"\x00\xF6\xF0\xF0\xFF\x33","Ӧ��ά����Կ DAMK_DF01"},
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

	//	ѡ��1PAY.SYS.DDF01
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
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret)
	{
		sprintf(strCmd,"ѡ�������ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	//	ɾ����Ƭ�ṹ
	callbackMessage("ɾ����Ƭ�ṹ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("800E000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ������Կ�ļ�
	callbackMessage("������Կ�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00000073F003C01AAFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	װ��������Կ
	callbackMessage("װ��������Կ");

	memset(strCmd,0x00,128);
	memcpy(strCmd,"80D4010015F9F0F0AA33",20);

	/*
	memset(sBuf,0x00,64);
	WatchDiversity(sBuf,szAPPID,INIT_KEY,TRUE);
	*/

	CMisc::ByteToString(INIT_KEY,16,strCmd+20);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);	
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	// װ��MF�µ�������Կ
	memset(szCMK,0x00,16);
	for(pkey=arr_mf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szCMK);
		if(ret) return ret;
	}

	//	����Ŀ¼�ļ�
	callbackMessage("����Ŀ¼�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00001072C0050F0AAFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}


	//	дĿ¼�ļ�
	callbackMessage("дĿ¼�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("00DC01040E6F0C840A5041592E4554432E5358",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ����0016�ļ�
	callbackMessage("����0016�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001607A80037F0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	д0016�ļ�
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

	//	����1001 
	callbackMessage("����1001 ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0100112381700AAAAFFFFFF5041592E4554432E5358",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

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

	//������Կ�ļ�
	callbackMessage("������Կ�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00000073F020095AAFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	װ��ADF�����е���Կ
	callbackMessage("װ��ADF��������Կ");

	for(pkey=arr_adf_keys;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szAMK);
		if(ret) return ret;
	}

	//	װ��ά����Կ
	memset(szAMK,0x00,16);
	for(pkey=arr_adf_amk;pkey->szKeyHeader;pkey++)
	{
		PRINTK("\nװ����Կ��%s",pkey->strDesc);
		ret = wd_load_key(bVer,szAPPID,szAPPID,pkey->bIndex,pkey->bHeaderLen,pkey->szKeyHeader,szAMK);
		if(ret) return ret;
	}

	// װ�ؿ�����Կ
	callbackMessage("װ�ؿ�����Կ");
		
	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80d401000D3AF0EF0133313233343536FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����0015�ļ�
	callbackMessage("����0015�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001507A80032F0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	д0015�ļ� 
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

	//	����0012�ļ� 
	callbackMessage("����0012�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001207280028F022FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	����0019�ļ�
	callbackMessage("����0019�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001907AC0250F0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	callbackMessage("����0019�ļ�");

	//	AA2900
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

	memset(sBuf,0x00,64);
	memcpy(sBuf,"\x04\xDC\x01\xCC\x2F\xAA\x29\x00",8);
	memset(sBuf+8,0xFF,40);
	
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

	//	B12900����B52900
	for(i=0xb1;i<0xb6;i++)
	{
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

		memset(sBuf,0x00,64);
		memcpy(sBuf,"\x04\xDC\x01\xCC\x2F\xB1\x29\x00",8);
		sBuf[2] = i-0xb1+2;
		sBuf[5] = i;
		memset(sBuf+8,0xFF,40);
		
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
	}

	//	C13D00����C23D00
	for(i=0xC1;i<0xC3;i++)
	{
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

		memset(sBuf,0x00,64);
		memcpy(sBuf,"\x04\xDC\x01\xCC\x43\xC1\x3D\x00",8);
		sBuf[2] = i-0xC1+7;
		sBuf[5] = i;
		memset(sBuf+8,0xFF,60);
		
		CmdWatchCalMac(68,sBuf,sRnd,szAMK,sBuf+68,true);

		memset(strCmd,0x00,512);
		CMisc::ByteToString(sBuf,72,strCmd);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	//	D15E00����D25E00
	for(i=0xD1;i<0xD3;i++)
	{
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

		memset(sBuf,0x00,64);
		memcpy(sBuf,"\x04\xDC\x01\xCC\x64\xD1\x5E\x00",8);
		sBuf[2] = i-0xD1+9;
		sBuf[5] = i;
		memset(sBuf+8,0xFF,93);
		
		CmdWatchCalMac(101,sBuf,sRnd,szAMK,sBuf+101,true);

		memset(strCmd,0x00,512);
		CMisc::ByteToString(sBuf,105,strCmd);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	����0002Ǯ���ļ�
	callbackMessage("����0002Ǯ���ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E000001E000200002F020862FFF0F00011F000FFFFFFFF0000000000000000000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����0018�ļ�
	callbackMessage("����0018�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00018072E321711EFFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}
	
	//	����0008�ļ� 
	callbackMessage("����0008�ļ� ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0000807280080F022FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����0009�ļ�
	callbackMessage("����0009�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0000907280200F0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����000A�ļ� 
	callbackMessage("����000A�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0000A072800FFF0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����001A�ļ�
	callbackMessage("����001A�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001A07AC0400F0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	for(i=0;i<34;i++)
	{
		sprintf(strCmd,"���� 001A�ļ���¼%d",szProviceFlag[i]);

		memset(strCmd,0x00,512);
		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd("0084000008",strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
		memset(sRnd,0x00,16);
		CMisc::StringToByte(strResp,sRnd);

		memset(sBuf,0x00,128);
		memcpy(sBuf,"\x04\xDC\x01\xd4\x22\x11\x1C\x00",8);
		memset(sBuf+8,0xff,27);
		sBuf[5] = szProviceFlag[i];

		CmdWatchCalMac(35,sBuf,sRnd,szAMK,sBuf+35,true);

		memset(strCmd,0x00,512);
		CMisc::ByteToString(sBuf,39,strCmd);

		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	//	����001B�ļ�
	callbackMessage("����001B�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001B072C0400F022FFFF",strResp);
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

		sprintf(strCmd,"00DC01041E%02X1C00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",szProviceFlag[i]);
		memset(strResp,0x00,128);
		ret = m_pReader->RunCmd(strCmd,strResp);
		if(ret)
		{
			sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
			callbackMessage(strCmd);
			return ret;
		}
	}

	//	����001C�ļ�
	callbackMessage("����001C�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001C072800FFF022FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����001D�ļ�
	callbackMessage("����001D�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0001D072800FFF022FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����000E�ļ�
	callbackMessage("����000E�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E0000E07A80046F0F0FFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	//	����ADF����
	callbackMessage("����ADF����");
	//	��������
	callbackMessage("��Ƭ��������");

	return 0;
}


//	��Ƭ���
int CWD3DESCard::clear(BYTE *elf15)
{
	int		ret;
	BYTE	szAPPID[8];
	
	BYTE	sRnd[16];
	char	strCmd[512],strResp[128];
	BYTE	sBuf[128];
	BYTE	INIT_KEY[16];

	memcpy(INIT_KEY,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);


	//	��Ƭ�汾�ź�Ӧ�����к�
	memcpy(szAPPID,elf15+12,8);

	//	ѡ��1PAY.SYS.DDF01
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
	strcpy(strCmd,"0082000008");
	CMisc::ByteToString(sBuf,8,strCmd+10);

	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(!ret) goto endl;


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
	WatchDiversity(sBuf,szAPPID,INIT_KEY,TRUE);

	memset(sBuf,0x00,64);
	des3_encode(INIT_KEY,sRnd,sBuf);

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


endl:


	//	ɾ����Ƭ�ṹ
	callbackMessage("ɾ����Ƭ�ṹ");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("800E000000",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	// ������Կ�ļ�
	callbackMessage("������Կ�ļ�");

	memset(strCmd,0x00,512);
	memset(strResp,0x00,128);
	ret = m_pReader->RunCmd("80E00000073F003C01AAFFFF",strResp);
	if(ret)
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	װ��������Կ
	callbackMessage("װ��������Կ");

	memset(strCmd,0x00,128);
	memcpy(strCmd,"80D4010015F9F0F0AA33",20);
	memcpy(INIT_KEY,"\x57\x41\x54\x43\x48\x44\x41\x54\x41\x54\x69\x6D\x65\x43\x4F\x53",16);
	CMisc::ByteToString(INIT_KEY,16,strCmd+20);

	memset(strResp,0x00,32);
	ret = m_pReader->RunCmd(strCmd,strResp);
	if(ret) 
	{
		sprintf(strCmd,"ʧ�ܣ�����:%04X",ret);
		callbackMessage(strCmd);	
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	return 0;
}
