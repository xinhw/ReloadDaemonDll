

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
			PRINTK("\n功能不支持：%04X",ph->wTransType);
			ret = ERR_UNSUPPORT;
			break;
	}
	updateDatagram();

	pszResult[0] = (BYTE)(wErrorCode>>8);
	pszResult[1] = (BYTE)(wErrorCode&0xff);

	GetSystemTimeAsFileTime(&ft1);
	int t0 = ft1.dwLowDateTime - ft0.dwLowDateTime;
	PRINTK("\n交易耗时(毫秒):%10.2f",(float)((0.1*t0)/10000));
	return ret;
}






/***********************************
3.1	操作员签到认证1031（一期不实现）
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
	//1.		返回代码	US	2	M	0     成功	其他  失败
	//2.		随机数	B	4		操作员卡认证码
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
3.2	充资认证1032
******************************************/
int		ClsReload::cmd_1032(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	BYTE bRandom[8],bOnlineSeqNo[4];
	BYTE bTransMoney[4];

	BYTE bTransType;
	BYTE bTerminalNo[6];
	BYTE MAC2[4];
	BYTE pszDID[8];			//	过程密钥因子：4字节随机数+2字节电子存折或电子钱包联机交易序号+\x80\x00
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
	//银行网点唯一编号	B	19		银行网点提供
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;
	//用户卡卡号	B	8		用户卡中获得
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;
	//IC卡伪随机数	B	4		圈存初始化获得
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;
	//电子存折或电子钱包联机交易序号 	B	4
	memcpy(bOnlineSeqNo,pszData+nIndex,4);
	nIndex=nIndex+4;
	//交易金额	B	4		充值金额
	memcpy(bTransMoney,pszData+nIndex,4);
	nIndex=nIndex+4;

	//交易类型标识	B	1
	memcpy(&bTransType,pszData+nIndex,1);
	nIndex=nIndex+1;

	 //终端机编号	B	6
	memcpy(bTerminalNo,pszData+nIndex,6); 
	nIndex=nIndex+6;

	//主机交易日期
	//主机交易时间	
	memcpy(bTransDateTime,pszData+nIndex,7);
	nIndex=nIndex+7;
	//////////////////////////////////////////////////////////////////////////

	
	//	（4字节随机数+2字节电子存折或电子钱包联机交易序号+8000）
	memcpy(pszDID,bRandom,4);
	memcpy(pszDID+4,bOnlineSeqNo+2,2);
	memcpy(pszDID+6,"\x80\x00",2);

	/*计算MAC给 返回代码	US	2	MAC2	B	4	剩余交易次数	US	2*/
	/*MAC2: 4字节交易金额+1字节交易类型标识+6字节终端机编号+4字节主机交易日期+3字节主机交易时间*/
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
	//1	返回代码	US	2		0     成功	其他  失败
	//2	MAC2	B	4		
	//3	剩余交易次数	US	2		
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

	//记录插入数据库
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
3.3	CPU卡二次发行认证1033
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
	//银行网点唯一编号	B	19		银行网点提供
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;

	//用户卡卡号	B	8		用户卡中获得
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;
	//IC卡伪随机数	B	4		圈存初始化获得
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;
	// 文件类别	B	1=
	memcpy(&bFileType,pszData+nIndex,1);   
	if(bFileType==0x01)		//	用户信息文件
		bKeyIndex = ENCODER_CPU_DAMK_MF_INDEX;
	else					//	发行信息文件
		bKeyIndex = ENCODER_CPU_DAMK_DF01_INDEX;
	nIndex=nIndex+1;

	//数据长度	B	1
	memcpy(&bFileLengh,pszData+nIndex,1);

	memcpy(sFileData,pszData+nIndex+1,bFileLengh);//文件信息数据	B	L
	//////////////////////////////////////////////////////////////////////////
	
	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(bKeyIndex,m_pszAPPID,bRandom,bFileLengh,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//加密数据得到ＭＡＣ
	//1	返回代码	US	2		0     成功；其他  失败
	//2	MAC	B	4		
	//3	剩余交易次数	US	2		
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
3.4	OBU二次发行认证1034
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
	//银行网点唯一编号	B	19		银行网点提供
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;
	
	//用户卡卡号	B	8		用户卡中获得
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;
	
	//IC卡伪随机数	B	4		圈存初始化获得
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;
	
	// 文件类别	B	1
	memcpy(&bFileType,pszData+nIndex,1);   
	if(bFileType==0x01)		//	系统信息文件
		bKeyIndex = ENCODER_OBU_DAMK_MF_INDEX;
	else
		bKeyIndex = ENCODER_OBU_DAMK_DF01_INDEX;	//	车辆信息文件
	nIndex=nIndex+1;

	//数据长度	B	1
	memcpy(&bFileLengh,pszData+nIndex,1);
	nIndex = nIndex +1;

	//文件信息数据	B	L
	memcpy( sFileData,pszData+nIndex,bFileLengh);
	//////////////////////////////////////////////////////////////////////////

	//加密数据得到MAC
	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(bKeyIndex,m_pszAPPID,bRandom,bFileLengh,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1	返回代码	US	2		0     成功其他  失败
	//2	MAC	B	4		
	//3	剩余交易次数	US	2		
	wSize = 8;
	memcpy(pszResult+2,MAC,4);
	memcpy(pph->pszMAC,MAC,4);
	pszResult[6] = (BYTE)(m_wRemainCount>>8);
	pszResult[7] = (BYTE)(m_wRemainCount&0xFF);
	//////////////////////////////////////////////////////////////////////////



	//记录插入数据库
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
3.5	修改储值卡有效期认证1035
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
	//银行网点唯一编号	B	19		银行网点提供
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;
	
	//用户卡卡号	B	8		用户卡中获得
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8; 
	
	//IC卡伪随机数	B	4		圈存初始化获得
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);    
	nIndex=nIndex+4;

	//数据长度	B	1
	memcpy(&bFileLengh,pszData+nIndex,1);
	nIndex=nIndex+1;

	//文件信息数据	B	L
	memcpy(sFileData,pszData+nIndex,bFileLengh);
	//////////////////////////////////////////////////////////////////////////

	//加密数据得到MAC
	memset(MAC,0x00,4);
	ret = pen->sjl_app_cal_cmd_mac(ENCODER_CPU_DAMK_DF01_INDEX,m_pszAPPID,bRandom,bFileLengh,sFileData,MAC);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		返回代码	US	2		0     成功	其他  失败
	//2.		MAC	B	4		
	//3.		剩余交易次数	US	2		
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
3.6	消费认证 1036
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
	BYTE pszDID[8];			//	过程密钥因子：4字节随机数+2字节电子存折或电子钱包联机交易序号+\x80\x00
	BYTE pszMACData[18];

	SYSTEMTIME st;
	BYTE	pszTransTime[8];

	if(wLenIn<59)
	{
		wErrorCode = ERR_APP_LENGTH;
		memset(pszResult+2,0x00,40);
		sprintf((char *)pszResult+2,"报文长度不足%d",wLenIn);	
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
	//银行网点唯一编号	B	19		银行网点提供
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;

	//用户卡卡号	B	8		用户卡中获得
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//IC卡伪随机数	B	4		圈存初始化获得
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4);     
	nIndex=nIndex+4;

	//电子存折或电子钱包联机交易序号 	B	4
	memcpy(bOnlineSeqNo,pszData+nIndex,4);
	nIndex=nIndex+4;

	//终端交易序号	B	2
	memcpy(bTerminalTranNo,pszData+nIndex,2);
	nIndex=nIndex+2;

	//交易前余额	B	4
	memcpy(bBerMoney,pszData+nIndex,4);
	nIndex=nIndex+4;

	//交易金额	B	4
	memcpy(bTransMoney,pszData+nIndex,4);
	nIndex=nIndex+4;
	
	//交易类型标识	B	1
	memcpy(&bTransType,pszData+nIndex,1);
	nIndex=nIndex+1;

	//终端机编号	B	6
	memcpy(bTerminalNo,pszData+nIndex,6);  
	nIndex=nIndex+6;
	
	//主机交易日期	B	4
	//主机交易时间	B	3
	//////////////////////////////////////////////////////////////////////////


	//	过程密钥因子：4字节随机数+2字节电子存折或电子钱包脱机交易序号+终端交易序号的最右两个字节
	memcpy(pszDID,bRandom,4);
	memcpy(pszDID+4,bOnlineSeqNo+2,2);
	memcpy(pszDID+6,bTerminalTranNo,2);

	//MAC1由卡中过程密钥对（4字节交易金额+1字节交易类型标识+6字节终端机编号+4字节终端交易日期+3字节终端交易时间）数据加密生成。
	memcpy(pszMACData,bTransMoney,4);
	memcpy(pszMACData+4,&bTransType,1);
	memcpy(pszMACData+5,bTerminalNo,6);
	memcpy(pszMACData+11,pszTransTime,7);

	/*计算MAC给 返回代码	US	2	MAC2	B	4	剩余交易次数	US	2*/
	memset(MAC2,0x00,4);
	ret =pen->sjl_app_cal_reload_mac(ENCODER_CPU_DPK1_INDEX,m_pszAPPID,pszDID,18,pszMACData,MAC2);
	if(ret)
	{
		wErrorCode = ERR_ENCODER;
	}

	//////////////////////////////////////////////////////////////////////////
	//1.		返回代码		US		2	必填	0     成功	其他  失败
	//2.		返回信息		Char	40	必填	出错信息
	//3.		主机交易日期	B		4	必填	ETC系统日期
	//4.		主机交易时间	B		3	必填	ETC系统时间
	//5.		MAC1			B		4	必填	用于执行消费
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
	//记录插入数据库
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
3.7	消费TAC验证 1037
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

	//	1037报文是否携带TAC
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
		sprintf((char *)pszResult+2,"报文长度不足%d",wLenIn);	
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//银行网点唯一编号	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19; 

	//用户卡卡号	B	8		用户卡中获得
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//交易金额	B	4
	memcpy(bTransMoney,pszData+nIndex,4); 
	nIndex=nIndex+4;

	//交易类型标识	B	1
	memcpy(&bTransType,pszData+nIndex,1);
	nIndex=nIndex+1;

	//终端机编号	B	6	
	memcpy(bTerminalNo,pszData+nIndex,6);
	nIndex=nIndex+6;	
	
	//终端交易序号	B	4
	memcpy(bTermTransNo,pszData+nIndex,4);
	nIndex=nIndex+4;
	
	//终端交易时间日期	B	7
	memcpy(bTransDateTime,pszData+nIndex,7);
	nIndex = nIndex + 7;

	//交易TAC码	B	4
	if(bDataGramWithTac)
	{
		memcpy(szTAC,pszData+nIndex,4);
		nIndex = nIndex + 4;
	}
	//////////////////////////////////////////////////////////////////////////


	//TAC用内部密钥DTK左右8位字节异或运算的结果对
	//（4字节交易金额+1字节交易类型标识+6字节终端机编号+4字节终端交易序号+4字节终端交易日期+3字节终端交易时间）数据加密生成。
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
	//1.		返回代码	US	2	必填	0     成功	其他  失败
	if(bDataGramWithTac)
	{
		wSize = 2;
		if(memcmp(pszTAC,szTAC,4))
		{
			PRINTK("\n上送TAC:%02X%02X%02X%02X 后台计算TAC:%02X%02X%02X%02X",
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
3.8	拆卸标志修改1038
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
	//1.银行网点唯一编号	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19;

	//2.OBU卡号	B	8
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//3.随机数	B	4
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4); 	
	nIndex=nIndex+4;	

	//4.		数据长度	B	1
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
	//1.		返回代码	US	2		0     成功其他  失败
	//2.		MAC	B	4		
	//3.		剩余交易次数	US	2		
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
3.9	PIN解锁认证1039
******************************************/
int		ClsReload::cmd_1039(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult)
{
	int ret =0;	
	int nIndex =0;

	BYTE bRandom[8];
	BYTE  bDataLen,sFileData[128];

	BYTE  MAC[4];

	//////////////////////////////////////////////////////////////////////////
	//1.银行网点唯一编号	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19; 	 

	//2.OBU卡号	B	8
	memcpy(m_pszAPPID,pszData+nIndex,8); 
	nIndex=nIndex+8;

	//3.随机数	B	4
	memset(bRandom,0x00,8);
	memcpy(bRandom,pszData+nIndex,4); 	
	nIndex=nIndex+4;	

	//4.		数据长度	B	1
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
	//1.		返回代码	US	2		0     成功	其他  失败
	//2.		MAC				4		
	//3.		剩余交易次数	US	2
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
3.10	CPU卡密钥获取1040
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
	//	输入内容
	//1.		银行网点唯一编号	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);

	//2.		分散因子			B	8	
	memcpy(m_pszAPPID,pszData+19,8);
	//3.		密钥编号			B	1
	bKeyID = pszData[27];
	//4.		认证MAC				B	4
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
	//返回代码			US	2		00 00
	//密钥密文			B	16		B9 F8 61 CB 01 2B E6 9E 41 3C ED 82 9D CF 6A 75
	memcpy(pszResult+2,pszSubKey,16);
	//剩余交易次数		US	2		00 00
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

//3.11	CPU卡二次发行补充文件认证1041
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
	//	输入内容
	//银行网点唯一编号	B	19
	memset(m_strBankID,0x00,20);
	memcpy(m_strBankID,pszData,19);
	nIndex=nIndex+19; 

	//城市代码	B	2	
	memset(szCityCode,0x00,2);
	memcpy(szCityCode,pszData+nIndex,2);
	nIndex=nIndex+2; 
	
	//卡唯一号	B	4		通行卡的唯一号
	memset(szSerialNo,0x00,4);
	memcpy(szSerialNo,pszData+nIndex,4);
	nIndex=nIndex+4; 

	//发行流水号	B	4	
	memset(szAuditNo,0x00,4);
	memcpy(szAuditNo,pszData+nIndex,4);
	nIndex=nIndex+4; 
	
	//卡认证码	B	4	
	memset(szCardMAC,0x00,4);
	memcpy(szCardMAC,pszData+nIndex,4);
	nIndex=nIndex+4; 
	
	//计算密钥个数	B	1	
	bKeyNum = pszData[nIndex];
	nIndex++;

	//扇区的特征码	B	L		
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

	//认证MAC	B	4		由该包体1-7数据通过操作员卡计算获得
	memcpy(pszMAC,pszData+nIndex,4);
	nIndex = nIndex + 4;
	//////////////////////////////////////////////////////////////////////////

	//	验证通行卡认证码
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

	//	计算卡片KeyA密钥
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

		//	一次性计算所有的密钥
		memset(pszResp,0x00,256);
		ret = pen->sjl_encode(szGIV,0,NULL,8*bKeyNum,pszBuf,pszResp);
		if(ret)
		{
			wErrorCode = ERR_ENCODER;
			return 0;
		}
		
		//8字节的结果转为6字节
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
	//1.		返回代码	US	2
	//2.		密钥个数	B	1
	pszResult[2] = bKeyNum;
	//3.		通行卡卡片KeyA密钥	B	L
	memcpy(pszResult+3,pszResp,bLen);

	//4.		剩余次数	US	2
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
			str="成功";
			break;
		case  1:
			str="其他失败";
			break;
		case  2:
			str="MAC错";
			break;
		case  3:
			str="内容错";
			break;
		case  4:
			str="系统故障";
			break;
		case  5:
			str="TAC错";
			break;
		default: str="TAC错";
			break;
	}
	return str;
}
















