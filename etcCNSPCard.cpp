/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/16 17:04:10

    Reversion:
        
-------------------------------------------------------------------------*/
// etcCNSPCard.cpp : Defines the entry point for the DLL application.
//


#include "dllinclude.hpp"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}



/*-------------------------------------------------------------------------
Function:		setCallbackFunc
Created:		2018-07-16 17:04:20
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	__stdcall	setCallbackFunc(CALLBACKFUNC p)
{
	pMyCallback = p;
}



void callbackMessage(char *strmsg)
{
	if(NULL==pMyCallback)
	{
		PRINTK("%s",strmsg);
	}
	else
	{
		pMyCallback(strlen(strmsg),strmsg);
	}
	return;
}


/***************************************************************************************/
/*				通用函数															   */
/***************************************************************************************/
/*1. 连接到在线密钥服务前置： Online Keys Service
	strip	[in]	前置的IP地址
	wport	[in]	前置的PORT
*/
/*-------------------------------------------------------------------------
Function:		connectOKS
Created:		2018-07-16 17:04:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall connectOKS(char *strip,WORD wport)
{
	int			ret;
	WSADATA		wsaData;  	

	//	WinSock2初始化
	WORD wSockVersion = MAKEWORD(2,1);
	ret = WSAStartup(wSockVersion,&wsaData);
	if(ret) 
	{
		PRINTK("\nWIN SOCKET2初始化失败！");
		return ret;
	}

	if(NULL!=ptransfer)
	{
		ptransfer->disconnect();
		delete ptransfer;
		ptransfer=NULL;
	}

	ptransfer = new CTcpTransfer();

	ret = ptransfer->init_socket();
	if(ret)
	{
		PRINTK("\ninit_socket失败:%d",ret);
		return ret;
	}

	ret = ptransfer->connect_server(wport,strip);
	if(ret)
	{
		PRINTK("\nconnect_server连接服务器%s:%d失败:%d",strip,wport,ret);
		delete ptransfer;
		ptransfer = NULL;
		return ret;
	}

	if(NULL==pcmd)
	{
		delete pcmd;
		pcmd = NULL;
	}
	pcmd = new ClsCommand(ptransfer);

	return 0;
}

/*2. 关闭到在线密钥服务前置的连接*/
/*-------------------------------------------------------------------------
Function:		disconnectOKS
Created:		2018-07-16 17:04:27
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall disconnectOKS()
{
	if(NULL!=ptransfer)
	{
		ptransfer->disconnect();
		delete ptransfer;
		ptransfer=NULL;
	}
	if(NULL==pcmd)
	{
		delete pcmd;
		pcmd = NULL;
	}

	return 0;
}

/*3. 打开读卡器
	nType	[in]	读卡器类型（支持多读卡器预留）
	ncom	[in]	端口号
	nbaud	[in]	波特率
*/
/*-------------------------------------------------------------------------
Function:		openReader
Created:		2018-07-16 17:04:30
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall openReader(int nType,int ncom,int nbaud)
{
	int ret;
	char strcom[32];

	if(NULL!=preader)
	{
		preader->Close();
		delete preader;
	}
	
	//	CPU卡读卡器（航天金卡）
	if(READER_TYPE_CPU_CARD==nType)
	{
		preader = new CAISINOReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader->Open(strcom,nbaud);
		return ret;
	}

	//	OBU读卡器（万集读卡器）
	if(READER_TYPE_OBU==nType)
	{
		preader = new CNXRsuReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader->Open(strcom,nbaud);
		return ret;
	}

	if(READER_TYPE_XIONGDI==nType)
	{
		preader = new CJTReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader->Open(strcom,nbaud);
		return ret;
	}


	return -1;
}



/*4. 关闭读卡器*/
/*-------------------------------------------------------------------------
Function:		closeReader
Created:		2018-07-16 17:04:33
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall closeReader()
{
	if(NULL!=preader)
	{
		preader->Close();
		delete preader;
		preader = NULL;
	}

	return 0;
}

/*5. 操作员签到认证
	strOperator	[in]	操作员卡号
*/
/*-------------------------------------------------------------------------
Function:		signIn
Created:		2018-07-16 17:04:36
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall signIn(char *strOperator)
{
	int ret;
	int n,i;
	BYTE szBuf[6];

	if(NULL==strOperator) return -1;
	if(NULL==ptransfer) return -2;
	if(NULL==pcmd) return -3;

	n = strlen(strOperator);
	if(n>12) n = 12;

	memset(szBuf,0x00,6);
	for(i=0;i<n;i=i+2) szBuf[i/2] = CMisc::ascToUC(strOperator[i])*0x10 + CMisc::ascToUC(strOperator[i+1]);

	pcmd->setTCPTransfer(ptransfer);

	ret = pcmd->cmd_1031(szBuf);
	return ret;
}


/***************************************************************************************/
/*				CPU函数																   */
/***************************************************************************************/
/*6. CPU卡复位
	szSNO	[in]	卡唯一号
	bATSLen	[in]	卡片ATS返回的长度
	szATS	[in]	卡片ATS信息
*/
int __stdcall cpuATS(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS)
{
	int ret;

	if(!validation(0)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	bATSLen = 0;
	ret = pcard->rats(szSNO,bATSLen,szATS);

	delete pcard;

	return ret;
}


/*7. 读CPU卡文件
	elf15		[out]	卡片发行基本数据文件
	elf16		[out]	持卡人基本数据文件
	dwRemain	[out]	余额
*/
/*-------------------------------------------------------------------------
Function:		cpuReadCardFiles
Created:		2018-07-16 17:04:41
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuReadCardFiles(BYTE *elf15,BYTE *elf16,DWORD &dwRemain)
{
	int ret;

	if(!validation(0)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->readCard(elf15,elf16,dwRemain);

	delete pcard;

	return ret;
}

/*
	8.	读取记录文件
	bFileID		[in]	文件标识，SFI
	bNo			[in]	记录号
	bLen		[in]	记录长度
	szRec		[out]	记录

  可以读取：0018,0017,0019文件
*/
int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec)
{
	int ret;

	if(!validation(0)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->readRecord(bFileID,bNo,bLen,szRec);

	delete pcard;

	return ret;

}

/*9. 用户卡一发
	szFile0015	[in]	0015文件内容
*/
/*-------------------------------------------------------------------------
Function:		cpuInit
Created:		2018-07-16 17:04:45
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuInit(BYTE *szFile0015)
{
	int ret;

	if(!validation(1)) return -1;

	CTYCPUCard *pcard = new CTYCPUCard(preader,pcmd);

	ret = pcard->init(szFile0015);

	delete pcard;

	return ret;

}


/*10. 更新持卡人基本数据文件
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	szFile		[in]	持卡人基本数据文件		
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateUserFile
Created:		2018-07-16 17:04:48
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->updateELF0016(bVer,szAPPID,szFile);

	delete pcard;

	return ret;
}


/*11. 更新卡发行基本数据文件
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	szFile		[in]	卡发行基本数据文件		
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateIssueFile
Created:		2018-07-16 17:04:51
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->updateELF0015(bVer,szAPPID,szFile);

	delete pcard;

	return ret;
}



/*12. 更新卡片有效期
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	szValidDate	[in]	新的有效期	
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateValidDate
Created:		2018-07-16 17:04:54
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->updateValidDate(bVer,szAPPID,szValidDate);

	delete pcard;

	return ret;
}

/*13. 圈存
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	dwAmount	[in]	圈存金额
	szDateTime	[in]	交易日期时间(BCD)
	wSeqNo		[out]	在线交易序号
	szTAC		[out]	TAC
*/
/*-------------------------------------------------------------------------
Function:		cpuCredit
Created:		2018-07-16 17:04:57
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuCredit(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,BYTE *szDateTime,BYTE *szDeviceNo,
						WORD &wSeqNo,BYTE *szTAC)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->credit(bVer,szAPPID,szDeviceNo,dwAmount,wSeqNo,szDateTime,szTAC);

	delete pcard;

	return ret;
}

/*14. 更新本地000E文件
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	szFile000E	[in]	000E文件内容
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateFile000E
Created:		2018-07-16 17:04:57
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateFile000E(BYTE bVer,BYTE *szAPPID,BYTE *szFile000E)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->updateELF000E(bVer,szAPPID,szFile000E);

	delete pcard;

	return ret;
}

/*15. 消费
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	dwAmount	[in]	消费金额
	dwAuditNo	[in]	终端交易序号
	szDateTime	[in]	交易日期时间(BCD)
	wSeqNo		[out]	脱机交易序号
	szTAC		[out]	TAC
*/
/*-------------------------------------------------------------------------
Function:		cpuPurchase
Created:		2018-07-16 17:05:00
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuPurchase(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,BYTE *szDeviceNo,
						WORD &wSeqNo,BYTE *szTAC)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->debit(bVer,szAPPID,szDeviceNo,dwAmount,dwAuditNo,szDateTime,wSeqNo,szTAC);

	delete pcard;

	return ret;
}

/*16. CPU卡验证TAC
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	dwAmount	[in]	消费金额
	bTransFlag	[in]	交易类型标识
	dwAuditNo	[in]	终端交易序号
	szDateTime	[in]	交易日期时间(BCD)
	szTAC		[in]	TAC
*/
/*-------------------------------------------------------------------------
Function:		cpuVerifyTAC
Created:		2018-07-16 17:05:04
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuVerifyTAC(BYTE bVer,BYTE *szAPPID,
						   DWORD dwAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD dwAuditNo,BYTE *szDateTime,
						   BYTE *szTAC)
{
	int ret;

	if(!validation(1)) return -1;

	ret = pcmd->cmd_1037(bVer,szAPPID,dwAmount,bTransFlag,szDeviceNo,dwAuditNo,szDateTime,szTAC);

	return ret;
}


/*
17. CPU卡重装PIN
	bVer		[in]	卡片版本号
	szAPPID		[in]	卡片应用序列号
	bPINLen		[in]	PIN的长度
	szPIN		[in]	新的PIN
*/
/*-------------------------------------------------------------------------
Function:		cpuReloadPIN
Created:		2018-07-16 17:05:07
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuReloadPIN(BYTE bVer,BYTE *szAPPID,BYTE bPINLen,BYTE *szPIN)
{
	int ret;

	if(!validation(1)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->reloadPIN(bVer,szAPPID,bPINLen,szPIN);

	delete pcard;

	return ret;
}


/***************************************************************************************/
/*				OBU函数	    														   */
/***************************************************************************************/


/*18. OBU一发
	elf01_mk	[in]	系统信息文件
	elf01_adf01	[in]	车辆信息文件
*/
/*-------------------------------------------------------------------------
Function:		obuInit
Created:		2018-07-16 17:05:13
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuInit(BYTE *elf01_mk,BYTE *elf01_adf01)
{
	int ret;

	if(!validation(1)) return -1;

	COBUCard *pcard = new COBUCard(preader,pcmd);

	ret = pcard->init(elf01_mk,elf01_adf01);

	delete pcard;

	return ret;
}




/*19. OBU读取信息
	elf01_mk	[in]	系统信息文件
	elf01_adf01	[in]	车辆信息文件
*/
/*-------------------------------------------------------------------------
Function:		obuRead
Created:		2018-07-16 17:05:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuRead(BYTE *elf01_mk,BYTE *elf01_adf01)
{
	int ret;

	if(!validation(1)) return -1;

	COBUCard *pcard = new COBUCard(preader,pcmd);

	ret = pcard->read_obu(elf01_mk,elf01_adf01);

	delete pcard;

	return ret;

}


/*20. OBU更新文件
	bVer	[in]	OBU合同版本号
	szAPPID	[in]	OBU合同序列号
	bLen	[in]	文件长度
	szFile	[out]	文件信息
*/
/*-------------------------------------------------------------------------
Function:		obuUpdateFile
Created:		2018-07-16 17:05:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bFileType,BYTE *szFile)
{
	int ret;

	if(!validation(1)) return -1;

	COBUCard *pcard = new COBUCard(preader,pcmd);

	if(bFileType==0x01)		//	系统信息文件
	{
		ret = pcard->update_mf_elf01(bVer,szAPPID,szFile);
	}
	else					//	更新车辆信息文件
	{
		ret = pcard->update_adf_elf01(bVer,szAPPID,szFile);
	}

	delete pcard;

	return ret;
}

/*21. OBU拆卸标志修改
	bVer	[in]	OBU合同版本号
	szAPPID	[in]	OBU合同序列号
	bFlag	[in]	OBU拆卸标志
*/
int __stdcall obuUpdateLoadFlag(BYTE bVer,BYTE *szAPPID,BYTE bFlag)
{
	int ret;

	if(!validation(1)) return -1;

	COBUCard *pcard = new COBUCard(preader,pcmd);

	ret = pcard->update_load_flag(bVer,szAPPID,bFlag);

	delete pcard;

	return ret;
}

/*-------------------------------------------------------------------------
Function:		validation
Created:		2018-07-28 10:58:19
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
bool validation(int nlevel)
{
	bool bOk = true;

	switch(nlevel)
	{
		case 1:
			if(NULL==pcmd)
			{
				PRINTK("\n未连接到前置！");
				bOk = false;
				break;
			}
		case 0:
			if(NULL==preader)
			{
				PRINTK("\n读卡器未打开！");
				bOk = false;
				break;
			}
		default:
			break;
	}

	return bOk;
}