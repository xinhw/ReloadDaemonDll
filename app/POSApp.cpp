

ClsReload::ClsReload()
{
	m_wRemainCount = 0;
}






ClsReload::~ClsReload()
{
}



int		ClsReload::process_request( PPACKAGEHEADER ph,const WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret;
	CProtocol		pt;	
	FILETIME	ft0,ft1;

	GetSystemTimeAsFileTime(&ft0);

	if(ph==NULL) return ERR_PARAM;

	memset(&m_ph,0x00,sizeof(PACKAGEHEADER));
	memcpy(&m_ph,ph,sizeof(PACKAGEHEADER));

	pt.print_package_header(&m_ph);

	getReferenceNo();
	insertDatagram(&m_ph);

	wErrorCode = 0x0000;
	switch(ph->wTransType) 
	{
		case 0x1031:
			ret = cmd_1031(wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1032:
			ret = cmd_1032(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1033:
			ret = cmd_1033(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1034:
			ret = cmd_1034(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1035:
			ret = cmd_1035(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1036:
			ret = cmd_1036(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1037:
			ret = cmd_1037(wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1038:
			ret = cmd_1038(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1039:
			ret = cmd_1039(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1040:
			ret = cmd_1040(ph,wLenIn,pszData,wSize,pszResult);
			break;
		case 0x1041:
			ret = cmd_1041(wLenIn,pszData,wSize,pszResult);
			break;
		case 0x2011:
			ret = cmd_2011(wLenIn,pszData,wSize,pszResult);
			break;
		default:
			PRINTK("\n���ܲ�֧�֣�%04X",ph->wTransType);
			ret = ERR_UNSUPPORT;
			break;
	}
	updateDatagram();

	pszResult[0] = (BYTE)(wErrorCode>>8);
	pszResult[1] = (BYTE)(wErrorCode&0xff);

	GetSystemTimeAsFileTime(&ft1);
	int t0 = ft1.dwLowDateTime - ft0.dwLowDateTime;
	PRINTK("\n���׺�ʱ(����):%10.2f",(float)((0.1*t0)/10000));
	return ret;
}






/***********************************
3.1	����Աǩ����֤1031��һ�ڲ�ʵ�֣�
******************************************/
int		ClsReload::cmd_1031(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{

	int			ret=0;
	CString		sSql="";
	char		strOperatorNo[13];

	if(wLenIn<6)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}



	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���	US	2	M	0     �ɹ�	����  ʧ��
	//2.		�����	B	4		����Ա����֤��
	wSize = 6;
	memcpy(pszResult+2,"\x00\x00\x00\x00",4);
	//////////////////////////////////////////////////////////////////////////

	memset(strOperatorNo,0x00,13);
	for( int i=0;i<6;i++) sprintf(strOperatorNo+2*i,"%02x", pszData[i]);

	sSql.Format("INSERT INTO  T_UserLog	(ProviderName,LogLevel,CreateDatetime,UserId,LogData)"
		"VALUES('%s','%s',%s,'%s',%s)",
		"1","1","getdate()",strOperatorNo,"getdate()");
	DBExecuteSQL(sSql);

	sSql.Format("INSERT INTO T_OperatorAuthentication(AuthId,OperatorCardId,RespOAC) "
		"VALUES( %d,'%s','%d')",
		nTransNo,strOperatorNo,ret);
	DBExecuteSQL(sSql);


	return 0;
}


/******************************************
3.2	������֤1032
******************************************/
int		ClsReload::cmd_1032(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	BYTE bRandom[8],bOnlineSeqNo[4];
	BYTE bTransMoney[4];

	BYTE bTransType;
	BYTE bTerminalNo[6];
	BYTE MAC2[4];
	BYTE pszDID[8];			//	������Կ���ӣ�4�ֽ������+2�ֽڵ��Ӵ��ۻ����Ǯ�������������+\x80\x00
	BYTE pszMACData[18];
	BYTE bTransDateTime[7];
	int ret,nIndex;

	if(wLenIn<53)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}

	nIndex =0;

	//////////////////////////////////////////////////////////////////////////
	//��������Ψһ���	B	19		���������ṩ
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;
	//�û�������	B	8		�û����л��
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;
	//IC��α�����	B	4		Ȧ���ʼ�����
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;
	//���Ӵ��ۻ����Ǯ������������� 	B	4
	memcpy(bOnlineSeqNo,pszData+nIndex,4);
	nIndex=nIndex+4;
	//���׽��	B	4		��ֵ���
	memcpy(bTransMoney,pszData+nIndex,4);
	nIndex=nIndex+4;

	//�������ͱ�ʶ	B	1
	memcpy(&bTransType,pszData+nIndex,1);
	nIndex=nIndex+1;

	 //�ն˻����	B	6
	memcpy(bTerminalNo,pszData+nIndex,6); 
	nIndex=nIndex+6;

	//������������
	//��������ʱ��	
	memcpy(bTransDateTime,pszData+nIndex,7);
	nIndex=nIndex+7;
	//////////////////////////////////////////////////////////////////////////

	
	//	��4�ֽ������+2�ֽڵ��Ӵ��ۻ����Ǯ�������������+8000��
	memcpy(pszDID,bRandom,4);
	memcpy(pszDID+4,bOnlineSeqNo+2,2);
	memcpy(pszDID+6,"\x80\x00",2);

	/*����MAC�� ���ش���	US	2	MAC2	B	4	ʣ�ཻ�״���	US	2*/
	/*MAC2: 4�ֽڽ��׽��+1�ֽڽ������ͱ�ʶ+6�ֽ��ն˻����+4�ֽ�������������+3�ֽ���������ʱ��*/
	memcpy(pszMACData,bTransMoney,4);
	memcpy(pszMACData+4,&bTransType,1);
	memcpy(pszMACData+5,bTerminalNo,6);
	memcpy(pszMACData+11,bTransDateTime,7);

	memset(MAC2,0x00,4);
	ret = pen->sjl_app_cal_reload_mac(ENCODER_CPU_DLK1_INDEX,m_pszAPPID,pszDID,18,pszMACData,MAC2);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1	���ش���	US	2		0     �ɹ�	����  ʧ��
	//2	MAC2	B	4		
	//3	ʣ�ཻ�״���	US	2		
	wSize = 8;
	memcpy(pszResult+2,MAC2,4);
	memcpy(pph->pszMAC,MAC2,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////

	int nBerMoney=0;
	int nOnlineSeqNo=0;
	int nTransMoney=0;	
	
	CMisc::Bytes2Int(bOnlineSeqNo,&nOnlineSeqNo);
	CMisc::Bytes2Int(bTransMoney,&nTransMoney);
	CTime tt=CTime::GetCurrentTime();
	CString date = tt.Format("%Y%m%d%H%M%S");

	//��¼�������ݿ�
	CString sSql="";	
	sSql.Format("INSERT INTO T_TransAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,OnlineTransSn,TransAmount,"
		"TransTypeCode,TerminalCode,TransDatetime,TransRemain,RespMAC2,"
		"RespRemainTimes,RespError,RespETCTransDatetime,RespMAC1)"
		"VALUES	(%d,'%s','%s','%s','%d',%d ,'%d','%s','%s',%d,'%s',	%d,'%d','%s','%s')"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),CMisc::toHexString(bRandom,4).c_str(),nOnlineSeqNo,nTransMoney
		,bTransType,CMisc::toHexString(bTerminalNo,6).c_str(),CMisc::toHexString(bTransDateTime,7).c_str(),nBerMoney,CMisc::toHexString(MAC2,4).c_str()
		,0,ret,date,CMisc::toHexString(MAC2,4).c_str()
		);
	ret= DBExecuteSQL(sSql);

	return 0;
}


/******************************************
3.3	CPU�����η�����֤1033
******************************************/
int		ClsReload::cmd_1033(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret =0;
	int nIndex =0;

	BYTE bRandom[8];
	BYTE bFileType;
	BYTE bFileLengh;
	BYTE sFileData[256];

	BYTE MAC[4];

	BYTE bKeyIndex = 0x00;
	
	if(wLenIn<29)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//��������Ψһ���	B	19		���������ṩ
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;

	//�û�������	B	8		�û����л��
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;
	//IC��α�����	B	4		Ȧ���ʼ�����
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;
	// �ļ����	B	1=
	memcpy(&bFileType,pszData+nIndex,1);   
	if(bFileType==0x01)		//	�û���Ϣ�ļ�
		bKeyIndex = ENCODER_CPU_DAMK_MF_INDEX;
	else					//	������Ϣ�ļ�
		bKeyIndex = ENCODER_CPU_DAMK_DF01_INDEX;
	nIndex=nIndex+1;

	//���ݳ���	B	1
	memcpy(&bFileLengh,pszData+nIndex,1);

	memcpy(sFileData,pszData+nIndex+1,bFileLengh);//�ļ���Ϣ����	B	L
	//////////////////////////////////////////////////////////////////////////
	
	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(bKeyIndex,m_pszAPPID,bRandom,bFileLengh,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//�������ݵõ��ͣ���
	//1	���ش���	US	2		0     �ɹ�������  ʧ��
	//2	MAC	B	4		
	//3	ʣ�ཻ�״���	US	2		
	wSize = 8;
	memcpy(pszResult+2,MAC,4);
	memcpy(pph->pszMAC,MAC,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////



	CString  sSql="";
	sSql.Format("INSERT INTO  T_CommonAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,DataLength,Data,"
		"MAC,RespMAC,RespRemainTimes,FileTypeCode)"
		"VALUES	(%d, '%s','%s','%s','%d','%s','%s','%s',%d,%d)"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),
		CMisc::toHexString(bRandom,4).c_str(),bFileLengh,CMisc::toHexString(sFileData,bFileLengh).c_str()
		," ",CMisc::toHexString(MAC,4).c_str(),0,bFileType);
	ret=DBExecuteSQL(sSql);
	return 0;

}


/******************************************
3.4	OBU���η�����֤1034
******************************************/
int		ClsReload::cmd_1034(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret =0;

	int nIndex =0;

	BYTE bRandom[8];
	BYTE bFileType;
	BYTE bFileLengh;
	BYTE sFileData[256];
	BYTE MAC[4],bKeyIndex;

	if(wLenIn<29)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//��������Ψһ���	B	19		���������ṩ
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;
	
	//�û�������	B	8		�û����л��
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;
	
	//IC��α�����	B	4		Ȧ���ʼ�����
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;
	
	// �ļ����	B	1
	memcpy(&bFileType,pszData+nIndex,1);   
	if(bFileType==0x01)		//	ϵͳ��Ϣ�ļ�
		bKeyIndex = ENCODER_OBU_DAMK_MF_INDEX;
	else
		bKeyIndex = ENCODER_OBU_DAMK_DF01_INDEX;	//	������Ϣ�ļ�
	nIndex=nIndex+1;

	//���ݳ���	B	1
	memcpy(&bFileLengh,pszData+nIndex,1);
	nIndex = nIndex +1;

	//�ļ���Ϣ����	B	L
	memcpy( sFileData,pszData+nIndex,bFileLengh);
	//////////////////////////////////////////////////////////////////////////

	//�������ݵõ�MAC
	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(bKeyIndex,m_pszAPPID,bRandom,bFileLengh,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1	���ش���	US	2		0     �ɹ�����  ʧ��
	//2	MAC	B	4		
	//3	ʣ�ཻ�״���	US	2		
	wSize = 8;
	memcpy(pszResult+2,MAC,4);
	memcpy(pph->pszMAC,MAC,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////



	//��¼�������ݿ�
	CString  sSql="";
	sSql.Format("INSERT INTO  T_CommonAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,DataLength,Data,"
		"MAC,RespMAC,RespRemainTimes,FileTypeCode)"
		"VALUES	(%d, '%s','%s','%s','%d','%s','%s','%s',%d,%d)"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),
		CMisc::toHexString(bRandom,4).c_str(),bFileLengh,CMisc::toHexString(sFileData,bFileLengh).c_str()
		," ",CMisc::toHexString(MAC,4).c_str(),0,bFileType);
	ret=DBExecuteSQL(sSql);

	return 0;

}

/******************************************
3.5	�޸Ĵ�ֵ����Ч����֤1035
******************************************/

int		ClsReload::cmd_1035(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret =0;
	int nIndex =0;

	BYTE bRandom[8];
	BYTE bFileLengh;
	BYTE sFileData[256];
	BYTE MAC[4];


	if(wLenIn<28)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//��������Ψһ���	B	19		���������ṩ
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;
	
	//�û�������	B	8		�û����л��
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8; 
	
	//IC��α�����	B	4		Ȧ���ʼ�����
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);    
	nIndex=nIndex+4;

	//���ݳ���	B	1
	memcpy(&bFileLengh,pszData+nIndex,1);
	nIndex=nIndex+1;

	//�ļ���Ϣ����	B	L
	memcpy(sFileData,pszData+nIndex,bFileLengh);
	//////////////////////////////////////////////////////////////////////////

	//�������ݵõ�MAC
	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(ENCODER_CPU_DAMK_DF01_INDEX,m_pszAPPID,bRandom,bFileLengh,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���	US	2		0     �ɹ�	����  ʧ��
	//2.		MAC	B	4		
	//3.		ʣ�ཻ�״���	US	2		
	wSize = 8;
	memcpy(pszResult+2,MAC,4);
	memcpy(pph->pszMAC,MAC,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////



	CString  sSql="";
	sSql.Format("INSERT INTO  T_CommonAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,DataLength,Data,"
		"MAC,RespMAC,RespRemainTimes,FileTypeCode)"
		"VALUES	(%d, '%s','%s','%s','%d','%s','%s','%s',%d,%d)"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),
		CMisc::toHexString(bRandom,4).c_str(),bFileLengh,CMisc::toHexString(sFileData,bFileLengh).c_str()
		," ",CMisc::toHexString(MAC,4).c_str(),0,0);
	ret=DBExecuteSQL(sSql);
	return 0;

}

/******************************************
3.6	������֤ 1036
******************************************/

int		ClsReload::cmd_1036(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret;
	int nIndex =0;

	BYTE bRandom[8];
	BYTE bOnlineSeqNo[4];
	BYTE bTerminalTranNo[2];

	BYTE bBerMoney[4];
	BYTE bTransMoney[4];

	BYTE bTransType;
	BYTE bTerminalNo[6];
	BYTE  MAC2[4];
	BYTE pszDID[8];			//	������Կ���ӣ�4�ֽ������+2�ֽڵ��Ӵ��ۻ����Ǯ�������������+\x80\x00
	BYTE pszMACData[18];

	SYSTEMTIME st;
	BYTE	pszTransTime[8];

	if(wLenIn<59)
	{
		wErrorCode = ERR_APP_LENGTH;
		memset(pszResult+2,0x00,40);
		sprintf((char *)pszResult+2,"���ĳ��Ȳ���%d",wLenIn);	
		return 0;
	}

	GetLocalTime(&st);
	memset(pszTransTime,0x00,8);
	pszTransTime[0] = CMisc::bcd(st.wYear/100);
	pszTransTime[1] = CMisc::bcd(st.wYear%100);
	pszTransTime[2] = CMisc::bcd((BYTE)st.wMonth);
	pszTransTime[3] = CMisc::bcd((BYTE)st.wDay);
	pszTransTime[4] = CMisc::bcd((BYTE)st.wHour);
	pszTransTime[5] = CMisc::bcd((BYTE)st.wMinute);
	pszTransTime[6] = CMisc::bcd((BYTE)st.wSecond);



	//////////////////////////////////////////////////////////////////////////
	//��������Ψһ���	B	19		���������ṩ
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;

	//�û�������	B	8		�û����л��
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//IC��α�����	B	4		Ȧ���ʼ�����
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;

	//���Ӵ��ۻ����Ǯ������������� 	B	4
	memcpy(bOnlineSeqNo,pszData+nIndex,4);
	nIndex=nIndex+4;

	//�ն˽������	B	2
	memcpy(bTerminalTranNo,pszData+nIndex,2);
	nIndex=nIndex+2;

	//����ǰ���	B	4
	memcpy(bBerMoney,pszData+nIndex,4);
	nIndex=nIndex+4;

	//���׽��	B	4
	memcpy(bTransMoney,pszData+nIndex,4);
	nIndex=nIndex+4;
	
	//�������ͱ�ʶ	B	1
	memcpy(&bTransType,pszData+nIndex,1);
	nIndex=nIndex+1;

	//�ն˻����	B	6
	memcpy(bTerminalNo,pszData+nIndex,6);  
	nIndex=nIndex+6;
	
	//������������	B	4
	//��������ʱ��	B	3
	//////////////////////////////////////////////////////////////////////////


	//	������Կ���ӣ�4�ֽ������+2�ֽڵ��Ӵ��ۻ����Ǯ���ѻ��������+�ն˽�����ŵ����������ֽ�
	memcpy(pszDID,bRandom,4);
	memcpy(pszDID+4,bOnlineSeqNo+2,2);
	memcpy(pszDID+6,bTerminalTranNo,2);

	//MAC1�ɿ��й�����Կ�ԣ�4�ֽڽ��׽��+1�ֽڽ������ͱ�ʶ+6�ֽ��ն˻����+4�ֽ��ն˽�������+3�ֽ��ն˽���ʱ�䣩���ݼ������ɡ�
	memcpy(pszMACData,bTransMoney,4);
	memcpy(pszMACData+4,&bTransType,1);
	memcpy(pszMACData+5,bTerminalNo,6);
	memcpy(pszMACData+11,pszTransTime,7);

	/*����MAC�� ���ش���	US	2	MAC2	B	4	ʣ�ཻ�״���	US	2*/
	memset(MAC2,0x00,4);
	ret =pen->sjl_app_cal_reload_mac(ENCODER_CPU_DPK1_INDEX,m_pszAPPID,pszDID,18,pszMACData,MAC2);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���		US		2	����	0     �ɹ�	����  ʧ��
	//2.		������Ϣ		Char	40	����	������Ϣ
	//3.		������������	B		4	����	ETCϵͳ����
	//4.		��������ʱ��	B		3	����	ETCϵͳʱ��
	//5.		MAC1			B		4	����	����ִ������
	wSize = 53;
	memcpy(pszResult+42,pszTransTime,7);
	memcpy(pszResult+49,MAC2,4);
	memcpy(pph->pszMAC,MAC2,4);
	//////////////////////////////////////////////////////////////////////////
	
	
	int nbOnlineSerNo=0;
	int nBerMoney=0;
	int nOnlineSeqNo=0;
	int nTransMoney=0;

	CMisc::Bytes2Int(bOnlineSeqNo,&nOnlineSeqNo);
	CMisc::Bytes2Int(bTransMoney,&nTransMoney);
	CMisc::Bytes2Int(bBerMoney,&nBerMoney);
	//��¼�������ݿ�
	CString  sSql="";
	sSql.Format("INSERT INTO T_TransAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,OnlineTransSn,TransAmount,"
		"TransTypeCode,TerminalCode,TransDatetime,TransRemain,RespMAC2,"
		"RespRemainTimes,RespError,RespETCTransDatetime,RespMAC1)"
		"VALUES	(%d,'%s','%s','%s','%d',%d ,'%d','%s','%s',%d,'%s',	%d,'%d','%s','%s')"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),CMisc::toHexString(bRandom,4).c_str(),nbOnlineSerNo,nTransMoney
		,bTransType,CMisc::toHexString(bTerminalNo,6).c_str(),CMisc::toHexString(pszTransTime,7).c_str(),nBerMoney,CMisc::toHexString(MAC2,4).c_str()
		,0,ret,CMisc::toHexString(pszTransTime,7).c_str(),CMisc::toHexString(MAC2,4).c_str());
	ret= DBExecuteSQL(sSql);
	return 0;
}


/******************************************
3.7	����TAC��֤ 1037
******************************************/
int		ClsReload::cmd_1037(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret=0;
	int nIndex =0;

	BYTE bTransMoney[4];
	BYTE bTransType;
	BYTE bTerminalNo[6];
	BYTE bTermTransNo[6];
	BYTE  bTransDateTime[7];

	BYTE  pszTACData[32];
	BYTE  pszTAC[4];
	
	BYTE	pszGIV[3];
	BYTE	szTAC[4];
	int		bDataGramWithTac = 0;
	BYTE	DATALENTH = 0;

	//	1037�����Ƿ�Я��TAC
	bDataGramWithTac = GetPrivateProfileInt("APP","WITHTAC1037",-1,INI_CONFIG);
	if(bDataGramWithTac==-1) 
	{
		WritePrivateProfileString("APP","WITHTAC1037","0",INI_CONFIG);
		bDataGramWithTac = 0;		
	}

	if(bDataGramWithTac)
	{
		DATALENTH = 49;
	}
	else
	{
		DATALENTH = 45;
	}



	if(wLenIn<DATALENTH)
	{
		wErrorCode = ERR_APP_LENGTH;
		memset(pszResult+2,0x00,40);
		sprintf((char *)pszResult+2,"���ĳ��Ȳ���%d",wLenIn);	
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//��������Ψһ���	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19; 

	//�û�������	B	8		�û����л��
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//���׽��	B	4
	memcpy(bTransMoney,pszData+nIndex,4); 
	nIndex=nIndex+4;

	//�������ͱ�ʶ	B	1
	memcpy(&bTransType,pszData+nIndex,1);
	nIndex=nIndex+1;

	//�ն˻����	B	6	
	memcpy(bTerminalNo,pszData+nIndex,6);
	nIndex=nIndex+6;	
	
	//�ն˽������	B	4
	memcpy(bTermTransNo,pszData+nIndex,4);
	nIndex=nIndex+4;
	
	//�ն˽���ʱ������	B	7
	memcpy(bTransDateTime,pszData+nIndex,7);
	nIndex = nIndex + 7;

	//����TAC��	B	4
	if(bDataGramWithTac)
	{
		memcpy(szTAC,pszData+nIndex,4);
		nIndex = nIndex + 4;
	}
	//////////////////////////////////////////////////////////////////////////


	//TAC���ڲ���ԿDTK����8λ�ֽ��������Ľ����
	//��4�ֽڽ��׽��+1�ֽڽ������ͱ�ʶ+6�ֽ��ն˻����+4�ֽ��ն˽������+4�ֽ��ն˽�������+3�ֽ��ն˽���ʱ�䣩���ݼ������ɡ�
	memset(pszTACData,0x00,32);
	memcpy(pszTACData,bTransMoney,4);
	pszTACData[4]=bTransType;
	memcpy(pszTACData+5,bTerminalNo,6);
	memcpy(pszTACData+11,bTermTransNo,4);
	memcpy(pszTACData+15,bTransDateTime,7);

	memset(pszGIV,0x00,3);
	pszGIV[2] = ENCODER_CPU_TAC_KEY_INDEX;

	memset(pszTAC,0x00,4);
	ret=pen->sjl_m1_cal_tac((BYTE *)pszGIV,1,m_pszAPPID,22,pszTACData,pszTAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���	US	2	����	0     �ɹ�	����  ʧ��
	if(bDataGramWithTac)
	{
		wSize = 2;
		if(memcmp(pszTAC,szTAC,4))
		{
			PRINTK("\n����TAC:%02X%02X%02X%02X ��̨����TAC:%02X%02X%02X%02X",
				szTAC[0],szTAC[1],szTAC[2],szTAC[3],
				pszTAC[0],pszTAC[1],pszTAC[2],pszTAC[3]);
			wErrorCode = ERR_TAC_CODE;
		}
		else
		{
			wErrorCode = 0x0000;
		}
	}
	else
	{
		wSize = 6;
		memcpy(pszResult+2,pszTAC,4);
	}
	//////////////////////////////////////////////////////////////////////////

	int nTermTransNo=0;
	int nTransMoney=0;

	CMisc::Bytes2Int(bTransMoney,&nTransMoney);
	CMisc::Bytes2Int(bTermTransNo,&nTermTransNo);

	CString		sSql="";	
	sSql.Format("INSERT INTO T_TACAuthentication"
		"(AuthId,BankSiteCode,UsedCardCode,TransAmount,TransTypeCode,TerminalCode,TerminalTransSn,TerminalTransDatetime,TransRemain)"
		"VALUES	(%d, '%s','%s','%d','%02x','%s','%d','%s',%d)"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),nTransMoney,bTransType,
		CMisc::toHexString(bTerminalNo,6).c_str(),nTermTransNo,CMisc::toHexString(bTransDateTime,6).c_str(),0);
	DBExecuteSQL(sSql);

	return 0;
}




/******************************************
3.8	��ж��־�޸�1038
******************************************/
int		ClsReload::cmd_1038(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{

	int nIndex =0;
	int ret =0;

	BYTE bRandom[8];
	BYTE bDataLen;
	BYTE sFileData[256];
	
	BYTE  MAC[4];

	//////////////////////////////////////////////////////////////////////////
	//1.��������Ψһ���	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;

	//2.OBU����	B	8
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//3.�����	B	4
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4); 	
	nIndex=nIndex+4;	

	//4.		���ݳ���	B	1
	memcpy(&bDataLen,pszData+nIndex,1); 
	nIndex=nIndex+1;

	memcpy(sFileData,pszData+nIndex,bDataLen);
	//////////////////////////////////////////////////////////////////////////

	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(ENCODER_OBU_DAMK_MF_INDEX,m_pszAPPID,bRandom,bDataLen,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���	US	2		0     �ɹ�����  ʧ��
	//2.		MAC	B	4		
	//3.		ʣ�ཻ�״���	US	2		
	wSize = 8;
	memcpy(pszResult+2,MAC,4);
	memcpy(pph->pszMAC,MAC,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////

	
	
	CString  sSql="";
	sSql.Format("INSERT INTO  T_CommonAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,DataLength,Data,"
		"MAC,RespMAC,RespRemainTimes,FileTypeCode)"
		"VALUES	(%d, '%s','%s','%s','%d','%s','%s','%s',%d,%d)"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),
		CMisc::toHexString(bRandom,4).c_str(),bDataLen,CMisc::toHexString(sFileData,bDataLen).c_str()
		," ",CMisc::toHexString(MAC,4).c_str(),0,0);
	ret=DBExecuteSQL(sSql);
	return 0;
}


/******************************************
3.9	PIN������֤1039
******************************************/
int		ClsReload::cmd_1039(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret =0;	
	int nIndex =0;

	BYTE bRandom[8];
	BYTE  bDataLen,sFileData[128];

	BYTE  MAC[4];

	//////////////////////////////////////////////////////////////////////////
	//1.��������Ψһ���	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19; 	 

	//2.OBU����	B	8
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//3.�����	B	4
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4); 	
	nIndex=nIndex+4;	

	//4.		���ݳ���	B	1
	memcpy(&bDataLen,pszData+nIndex,1); 
	nIndex=nIndex+1;	

	memcpy(sFileData,pszData+nIndex,bDataLen);
	//////////////////////////////////////////////////////////////////////////

	memset(MAC,0x00,4);
	ret = pen->sjl_app_verify_tac(ENCODER_CPU_RELOAD_PIN_INDEX,m_pszAPPID,bDataLen-5,sFileData+5,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���	US	2		0     �ɹ�	����  ʧ��
	//2.		MAC				4		
	//3.		ʣ�ཻ�״���	US	2
	wSize = 8;
	memcpy(pszResult+2,MAC,4);
	memcpy(pph->pszMAC,MAC,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////

	CString  sSql="";
	sSql.Format("INSERT INTO  T_CommonAuthentication(AuthId,BankSiteCode,UsedCardCode,RandomNum,DataLength,Data,"
		"MAC,RespMAC,RespRemainTimes,FileTypeCode)"
		"VALUES	(%d, '%s','%s','%s','%d','%s','%s','%s',%d,%d)"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),
		CMisc::toHexString(bRandom,4).c_str(),bDataLen,CMisc::toHexString(sFileData,bDataLen).c_str()
		," ",CMisc::toHexString(MAC,4).c_str(),0,0);
	ret=DBExecuteSQL(sSql);

	return 0;
}

/******************************************
3.10	CPU����Կ��ȡ1040
******************************************/

int		ClsReload::cmd_1040(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	BYTE		bKeyID;
	BYTE		pszMAC[4];
	BYTE		pszSubKey[16];
	WORD		ret;

	if(wLenIn<28)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//	��������
	//1.		��������Ψһ���	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);

	//2.		��ɢ����			B	8	
	memcpy(m_pszAPPID,pszData+19,8);
	//3.		��Կ���			B	1
	bKeyID = pszData[27];
	//4.		��֤MAC				B	4
	memcpy(pszMAC,pszData+28,4);
	//////////////////////////////////////////////////////////////////////////

	memset(pszSubKey,0x00,16);
	ret = pen->cmd_diversify_key(0x00,bKeyID,m_pszAPPID,pszSubKey);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	wSize = 20;
	//////////////////////////////////////////////////////////////////////////
	//���ش���			US	2		00 00
	//��Կ����			B	16		B9 F8 61 CB 01 2B E6 9E 41 3C ED 82 9D CF 6A 75
	memcpy(pszResult+2,pszSubKey,16);
	//ʣ�ཻ�״���		US	2		00 00
	pszResult[18] = (BYTE)(m_wRemainCount>>8);
	pszResult[19] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////
	
	CString  sSql="";
	sSql.Format("INSERT INTO  T_AcquiringKeyAuthentication(AuthId,BankSiteCode,UsedCardCode,KeyTypeCode,MAC,RespEncryptedKey)"
		"VALUES	(%d, '%s','%s','%02X','%s','%s')"
		,nTransNo,m_strBankID,CMisc::toHexString(m_pszAPPID,8).c_str(),bKeyID,CMisc::toHexString(pszMAC,4).c_str(),CMisc::toHexString(pszSubKey,8).c_str());
	ret=DBExecuteSQL(sSql);


	return 0;
}

//3.11	CPU�����η��в����ļ���֤1041
int		ClsReload::cmd_1041(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	return 0;
}


int		ClsReload::cmd_2011(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	BYTE		pszMAC[4];
	WORD		ret;
	BYTE		i,bKeyNum;
	BYTE		ucSector[16];
	BYTE		pszBuf[128],pszResp[256];
	BYTE		szCityCode[2],szSerialNo[4],szAuditNo[4],szCardMAC[4];
	BYTE		bLen;
	BYTE		szKey[16];
	BYTE		nIndex,szGIV[3];
	
	wSize = 2;

	if(wLenIn<30)
	{
		wErrorCode = ERR_APP_LENGTH;
		return 0;
	}

	nIndex = 0x00;
	//////////////////////////////////////////////////////////////////////////
	//	��������
	//��������Ψһ���	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19; 

	//���д���	B	2	
	memset(szCityCode,0x00,2);
	memcpy(szCityCode,pszData+nIndex,2);
	nIndex=nIndex+2; 
	
	//��Ψһ��	B	4		ͨ�п���Ψһ��
	memset(szSerialNo,0x00,4);
	memcpy(szSerialNo,pszData+nIndex,4);
	nIndex=nIndex+4; 

	//������ˮ��	B	4	
	memset(szAuditNo,0x00,4);
	memcpy(szAuditNo,pszData+nIndex,4);
	nIndex=nIndex+4; 
	
	//����֤��	B	4	
	memset(szCardMAC,0x00,4);
	memcpy(szCardMAC,pszData+nIndex,4);
	nIndex=nIndex+4; 
	
	//������Կ����	B	1	
	bKeyNum = pszData[nIndex];
	nIndex++;

	//������������	B	L		
	if(bKeyNum>16) 
	{
		wErrorCode = ERR_M1_KEY_NUM;
		return 0;
	}
	if(bKeyNum>0)
	{
		memset(ucSector,0x00,16);
		memcpy(ucSector,pszData+nIndex,bKeyNum);
		nIndex = nIndex + bKeyNum;
	}

	//��֤MAC	B	4		�ɸð���1-7����ͨ������Ա��������
	memcpy(pszMAC,pszData+nIndex,4);
	nIndex = nIndex + 4;
	//////////////////////////////////////////////////////////////////////////

	//	��֤ͨ�п���֤��
	memset(pszBuf,0x00,128);
	memcpy(pszBuf,szCityCode,2);
	memcpy(pszBuf+2,szSerialNo,4);
	memcpy(pszBuf+6,szAuditNo+2,2);

	memset(szGIV,0x00,3);
	szGIV[2] = ENCODER_M1_KEYA_INDEX;

	ret = pen->sjl_encode(szGIV,0,NULL,8,pszBuf,pszBuf+64);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
		return 0;
	}

	if(memcmp(szCardMAC,pszBuf+64,4))
	{
		wErrorCode = ERR_M1_CARD_MAC;
		return 0;
	}

	//	���㿨ƬKeyA��Կ
	bLen = 0;
	if(bKeyNum>0)
	{
		memset(pszBuf,0x00,128);
		for(i=0;i<bKeyNum;i++)
		{
			memcpy(pszBuf+i*8,szSerialNo,4);
			memcpy(pszBuf+i*8+4,szAuditNo+2,2);
			memcpy(pszBuf+i*8+6,szCardMAC,1);
			pszBuf[i*8+7] = ucSector[i];
		}

		//	һ���Լ������е���Կ
		memset(pszResp,0x00,256);
		ret = pen->sjl_encode(szGIV,0,NULL,8*bKeyNum,pszBuf,pszResp);
		if(ret)
		{
			wErrorCode = ERR_ENCODER;
			return 0;
		}
		
		//8�ֽڵĽ��תΪ6�ֽ�
		memset(pszBuf,0x00,128);
		for(i=0;i<bKeyNum;i++) memcpy(pszBuf+i*6,pszResp+i*8,6);

		bLen = bKeyNum*6;

		i = (BYTE)(8-bLen%8);
		memcpy(pszBuf+bLen,"\x80\x00\x00\x00\x00\x00\x00\x00",i);
		bLen = bLen + i;

		memset(szKey,0x00,16);
		for(i=0;i<bLen;i=i+8) des3_decode(szKey,pszBuf+i,pszResp+i);
	}


	wSize = 5+bLen;
	//////////////////////////////////////////////////////////////////////////
	//1.		���ش���	US	2
	//2.		��Կ����	B	1
	pszResult[2] = bKeyNum;
	//3.		ͨ�п���ƬKeyA��Կ	B	L
	memcpy(pszResult+3,pszResp,bLen);

	//4.		ʣ�����	US	2
	pszResult[bLen+3] = (BYTE)(m_wRemainCount>>8);
	pszResult[bLen+4] = (BYTE)(m_wRemainCount&0xFF);


	return 0;
}



CString  ClsReload::retMessage( int  i)
{
	CString str="";
	switch(i)
	{
		case  0:
			str="�ɹ�";
			break;
		case  1:
			str="����ʧ��";
			break;
		case  2:
			str="MAC��";
			break;
		case  3:
			str="���ݴ�";
			break;
		case  4:
			str="ϵͳ����";
			break;
		case  5:
			str="TAC��";
			break;
		default: str="TAC��";
			break;
	}
	return str;
}
















