/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/16 17:04:10

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
	int i;

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			PRINTK("\nDLL��ʼ��(DLL_PROCESS_ATTACH)");			
			for(i=0;i<MAX_READER_NUM;i++)
			{				
				ptransfer[i] = NULL;					
				pcmd[i] = NULL;				
				preader[i] = NULL;
			}
			
			memset(strBankID,0x00,20);
			memset(strAgentCode,0x00,7);

			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			PRINTK("\nDLL�˳�����(DLL_PROCESS_DETACH)");
			
			for(i=0;i<MAX_READER_NUM;i++)
			{
				closeReader(i);
				disconnectOKS(i);
			}
			
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

/*-------------------------------------------------------------------------
Function:		getDllVersion
Created:		2018-07-16 17:04:20
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void	__stdcall	getDllVersion(char *strVer)
{
	strcpy(strVer,DLL_VERSION);
	return;
}

/*-------------------------------------------------------------------------
Function:		callbackMessage
Created:		2018-07-16 17:04:20
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void callbackMessage(char *strmsg)
{
	if(NULL==pMyCallback)
	{
		PRINTK("\n%s",strmsg);
	}
	else
	{
		pMyCallback(strlen(strmsg),strmsg);
	}
	return;
}


/***************************************************************************************/
/*				ͨ�ú���															   */
/***************************************************************************************/
/*1. ���ӵ�������Կ����ǰ�ã� Online Keys Service
	strip	[in]	ǰ�õ�IP��ַ
	wport	[in]	ǰ�õ�PORT
*/
/*-------------------------------------------------------------------------
Function:		connectOKS
Created:		2018-07-16 17:04:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall connectOKS(char *strip,WORD wport,int ncom)
{
	int			ret;
	WSADATA		wsaData;  	

	PRINTK("\r\n�������˿�(connectOKS)��%d",ncom);

	if(ncom>=MAX_READER_NUM) return ERR_OVER_MAX_COM;

	//	WinSock2��ʼ��
	WORD wSockVersion = MAKEWORD(2,1);
	ret = WSAStartup(wSockVersion,&wsaData);
	if(ret) 
	{
		PRINTK("\nWIN SOCKET2��ʼ��ʧ�ܣ�");
		return ret;
	}

	if(NULL!=ptransfer[ncom])
	{
		ptransfer[ncom]->disconnect();
		delete ptransfer[ncom];
		ptransfer[ncom]=NULL;
	}

	ptransfer[ncom] = new CTcpTransfer();
	ret = ptransfer[ncom]->init_socket();
	if(ret)
	{
		PRINTK("\ninit_socketʧ��:%d",ret);
		return ret;
	}

	ret = ptransfer[ncom]->connect_server(wport,strip);
	if(ret)
	{
		PRINTK("\nconnect_server���ӷ�����%s:%dʧ��:%d",strip,wport,ret);
		delete ptransfer[ncom];
		ptransfer[ncom] = NULL;
		return ret;
	}

	if(NULL!=pcmd[ncom])
	{
		delete pcmd[ncom];
		pcmd[ncom] = NULL;
	}
	pcmd[ncom] = new ClsCommand(ptransfer[ncom]);

	if(strlen(strBankID)==0) strcpy(strBankID,"5201301101512061652");
	pcmd[ncom]->setBankID(strBankID);
	
	if(strlen(strAgentCode)==0) strcpy(strAgentCode,"668801");
	pcmd[ncom]->setAgentCode(strAgentCode);

	return 0;
}

/*2. �رյ�������Կ����ǰ�õ�����*/
/*-------------------------------------------------------------------------
Function:		disconnectOKS
Created:		2018-07-16 17:04:27
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall disconnectOKS(int ncom)
{
	PRINTK("\r\n�������˿�(disconnectOKS)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(NULL!=ptransfer[n])
	{
		ptransfer[n]->disconnect();
		delete ptransfer[n];
		ptransfer[n]=NULL;
	}
	if(NULL!=pcmd[n])
	{
		delete pcmd[n];
		pcmd[n] = NULL;
	}

	return 0;
}

/*3. �򿪶�����
	nType	[in]	���������ͣ�֧�ֶ������Ԥ����
	ncom	[in]	�˿ں�
	nbaud	[in]	������
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

	if(ncom>=MAX_READER_NUM) return ERR_OVER_MAX_COM;

	gnDefaultCom = ncom;
	PRINTK("\r\n�������˿�(openReader)��%d",ncom);

	if(NULL!=preader[ncom])
	{
		preader[ncom]->Close();
		delete preader[ncom];
	}
	
	//	CPU��������������𿨣�
	if(READER_TYPE_CPU_CARD==nType)
	{
		preader[ncom] = new CAISINOReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader[ncom]->Open(strcom,nbaud);
		return ret;
	}

	//	OBU���������򼯶�������
	if(READER_TYPE_OBU==nType)
	{
		preader[ncom] = new CNXRsuReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader[ncom]->Open(strcom,nbaud);
		return ret;
	}

	//	�����۵�CPU��������
	if(READER_TYPE_XIONGDI==nType)
	{
		preader[ncom] = new CJTReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader[ncom]->Open(strcom,nbaud);
		return ret;
	}

	//	����OBU�˿���
	if(READER_TYPE_JINYI==nType)
	{
		preader[ncom] = new CJYRsuReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader[ncom]->Open(strcom,nbaud);
		return ret;
	}

	return -1;
}



/*4. �رն�����*/
/*-------------------------------------------------------------------------
Function:		closeReader
Created:		2018-07-16 17:04:33
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall closeReader(int ncom)
{
	PRINTK("\r\n�������˿�(closeReader)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(NULL!=preader[n])
	{
		preader[n]->Close();
		delete preader[n];
		preader[n] = NULL;
	}

	return 0;
}

/*5. ����Աǩ����֤
	strOperator	[in]	����Ա����
*/
/*-------------------------------------------------------------------------
Function:		signIn
Created:		2018-07-16 17:04:36
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall signIn(char *strOperator,int ncom)
{
	int ret;
	int n,i;
	BYTE szBuf[6];

	PRINTK("\r\n�������˿�(signIn)��%d",ncom);

	int icom = ncom%MAX_READER_NUM;

	if(NULL==strOperator) return -1;
	if(NULL==ptransfer[icom]) return -2;
	if(NULL==pcmd[icom]) return -3;

	n = strlen(strOperator);
	if(n>12) n = 12;

	memset(szBuf,0x00,6);
	for(i=0;i<n;i=i+2) szBuf[i/2] = CMisc::ascToUC(strOperator[i])*0x10 + CMisc::ascToUC(strOperator[i+1]);

	ret = pcmd[icom]->cmd_1031(szBuf);
	return ret;
}


/***************************************************************************************/
/*				CPU����																   */
/***************************************************************************************/
/*6. CPU����λ
	szSNO	[in]	��Ψһ��
	bATSLen	[in]	��ƬATS���صĳ���
	szATS	[in]	��ƬATS��Ϣ
*/
int __stdcall cpuATS(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuATS)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(0,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	bATSLen = 0;
	ret = pcard->rats(szSNO,bATSLen,szATS);

	delete pcard;

	return ret;
}


/*7. ��CPU���ļ�
	elf15		[out]	��Ƭ���л��������ļ�
	elf16		[out]	�ֿ��˻��������ļ�
	dwRemain	[out]	���
*/
/*-------------------------------------------------------------------------
Function:		cpuReadCardFiles
Created:		2018-07-16 17:04:41
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuReadCardFiles(BYTE *elf15,BYTE *elf16,DWORD &dwRemain,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuReadCardFiles)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(0,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->readCard(elf15,elf16,dwRemain);

	delete pcard;

	return ret;
}


/*-------------------------------------------------------------------------
Function:		cpuReadAdfFile
Created:		2018-08-03 15:28:51
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuReadAdfFile(BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE *szFile,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuReadAdfFile)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(0,n)) return -1;


	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->readAdfFile(bFileID,bOffset,bLength,szFile);

	delete pcard;

	return ret;
}


/*
	8.	��ȡ��¼�ļ�
	bFileID		[in]	�ļ���ʶ��SFI
	bNo			[in]	��¼��
	bLen		[in]	��¼����
	szRec		[out]	��¼

  ���Զ�ȡ��0018,0017,0019�ļ�
*/
int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuReadCardRecord)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(0,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->readRecord(bFileID,bNo,bLen,szRec);

	delete pcard;

	return ret;

}

/*9. �û���һ��
	szFile0015	[in]	0015�ļ�����
*/
/*-------------------------------------------------------------------------
Function:		cpuInit
Created:		2018-07-16 17:04:45
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuInit(BYTE *szFile0015,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuInit)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard;

	ret = getCardType();
	if(ret == CARD_TYPE_TY_SM4)
		pcard = new CTYCPUCard(preader[n],pcmd[n]);
	else
		pcard = new CWD3DESCard(preader[n],pcmd[n]);

	ret = pcard->init(szFile0015);

	delete pcard;

	return ret;

}


int __stdcall cpuClear(BYTE *szFile0015,int ncom )
{
	int ret;

	PRINTK("\r\n�������˿�(cpuClear)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard;

	ret = getCardType();
	if(ret == CARD_TYPE_TY_SM4)
		pcard = new CTYCPUCard(preader[n],pcmd[n]);
	else
		pcard = new CWD3DESCard(preader[n],pcmd[n]);

	ret = pcard->clear(szFile0015);

	delete pcard;

	return ret;
}

/*10. ���³ֿ��˻��������ļ�
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	szFile		[in]	�ֿ��˻��������ļ�		
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateUserFile
Created:		2018-07-16 17:04:48
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuUpdateUserFile)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->updateELF0016(bVer,szAPPID,szFile);

	delete pcard;

	return ret;
}


/*11. ���¿����л��������ļ�
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	szFile		[in]	�����л��������ļ�		
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateIssueFile
Created:		2018-07-16 17:04:51
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuUpdateIssueFile)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->updateELF0015(bVer,szAPPID,szFile);

	delete pcard;

	return ret;
}



/*12. ���¿�Ƭ��Ч��
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	szValidDate	[in]	�µ���Ч��	
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateValidDate
Created:		2018-07-16 17:04:54
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuUpdateValidDate)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->updateValidDate(bVer,szAPPID,szValidDate);

	delete pcard;

	return ret;
}

/*13. Ȧ��
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	dwAmount	[in]	Ȧ����
	szDateTime	[in]	��������ʱ��(BCD)
	wSeqNo		[out]	���߽������
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
						WORD &wSeqNo,BYTE *szTAC,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuCredit)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->credit(bVer,szAPPID,szDeviceNo,dwAmount,wSeqNo,szDateTime,szTAC);

	delete pcard;

	return ret;
}

/*14. ���±���000E�ļ�
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	szFile000E	[in]	000E�ļ�����
*/
/*-------------------------------------------------------------------------
Function:		cpuUpdateFile000E
Created:		2018-07-16 17:04:57
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				
-------------------------------------------------------------------------*/
int __stdcall cpuUpdateFile000E(BYTE bVer,BYTE *szAPPID,BYTE *szFile000E,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuUpdateFile000E)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->updateELF000E(bVer,szAPPID,szFile000E);

	delete pcard;

	return ret;
}

/*15. ����
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	dwAmount	[in]	���ѽ��
	dwAuditNo	[in]	�ն˽������
	szDateTime	[in]	��������ʱ��(BCD)
	wSeqNo		[out]	�ѻ��������
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
						WORD &wSeqNo,BYTE *szTAC,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuPurchase)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->debit(bVer,szAPPID,szDeviceNo,dwAmount,dwAuditNo,szDateTime,wSeqNo,szTAC);

	delete pcard;

	return ret;
}

/*16. CPU����֤TAC
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	dwAmount	[in]	���ѽ��
	bTransFlag	[in]	�������ͱ�ʶ
	dwAuditNo	[in]	�ն˽������
	szDateTime	[in]	��������ʱ��(BCD)
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
						   BYTE *szTAC,int ncom)
{
	int ret;
	
	PRINTK("\r\n�������˿�(cpuVerifyTAC)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(NULL==pcmd[n])
	{
		PRINTK("\nCOM[%d]δ���ӵ�ǰ�ã�",n);
		return ERR_SOCKET_INVALID;
	}

	ret = pcmd[n]->cmd_1037(bVer,szAPPID,dwAmount,bTransFlag,szDeviceNo,dwAuditNo,szDateTime,szTAC);

	return ret;
}


/*
17. CPU����װPIN
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
	bPINLen		[in]	PIN�ĳ���
	szPIN		[in]	�µ�PIN
*/
/*-------------------------------------------------------------------------
Function:		cpuReloadPIN
Created:		2018-07-16 17:05:07
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuReloadPIN(BYTE bVer,BYTE *szAPPID,BYTE bPINLen,BYTE *szPIN,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(cpuReloadPIN)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	CCPUCardBase *pcard = new CCPUCardBase(preader[n],pcmd[n]);

	ret = pcard->reloadPIN(bVer,szAPPID,bPINLen,szPIN);

	delete pcard;

	return ret;
}


/***************************************************************************************/
/*				OBU����	    														   */
/***************************************************************************************/


/*18. OBUһ��
	elf01_mk	[in]	ϵͳ��Ϣ�ļ�
	elf01_adf01	[in]	������Ϣ�ļ�
*/
/*-------------------------------------------------------------------------
Function:		obuInit
Created:		2018-07-16 17:05:13
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuInit(BYTE *elf01_mk,BYTE *elf01_adf01,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuInit)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	ret = pcard->init(elf01_mk,elf01_adf01);

	delete pcard;

	return ret;
}

/*-------------------------------------------------------------------------
Function:		obuPreInit
Created:		2018-07-16 17:05:13
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuPreInit(WORD wDFID,BYTE *elf01_mk,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuPreInit)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	ret = pcard->preInit(wDFID,elf01_mk);

	delete pcard;

	return ret;
}

/*-------------------------------------------------------------------------
Function:		obuGetUID
Created:		2018-07-16 17:05:13
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				��ȡOBUоƬ���к�
-------------------------------------------------------------------------*/
int	__stdcall obuGetUID(BYTE *szUID,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuGetUID)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	ret = pcard->getOBUUID(szUID);

	delete pcard;

	return ret;
}



/*19. OBU��ȡ��Ϣ
	elf01_mk	[in]	ϵͳ��Ϣ�ļ�
*/
/*-------------------------------------------------------------------------
Function:		obuRead
Created:		2018-07-16 17:05:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuRead(BYTE *elf01_mk,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuRead)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	ret = pcard->read_obu(elf01_mk);

	delete pcard;

	return ret;

}

/*
	ͨ��PSAM�����ܳ�����Ϣ�ļ���
	bNode		[in]	PSAM�Ľڵ��
	bVer		[in]	PSAM���н�����Կ�İ汾
	szPlainFile	[out]	���ܵĳ�����Ϣ�ļ�
*/
/*-------------------------------------------------------------------------
Function:		obuReadVehicleFile
Created:		2018-07-16 17:05:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuReadVehicleFile(BYTE bNode,BYTE bVer,BYTE *szPlainFile,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuReadVehicleFile)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	ret = pcard->read_vechile_file(bNode,bVer,szPlainFile);

	delete pcard;

	return ret;
}

/*20. OBU�����ļ�
	bVer	[in]	OBU��ͬ�汾��
	szAPPID	[in]	OBU��ͬ���к�
	bLen	[in]	�ļ�����
	szFile	[out]	�ļ���Ϣ
*/
/*-------------------------------------------------------------------------
Function:		obuUpdateFile
Created:		2018-07-16 17:05:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bFileType,BYTE *szFile,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuUpdateFile)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	if(bFileType==0x01)		//	ϵͳ��Ϣ�ļ�
	{
		ret = pcard->update_mf_elf01(bVer,szAPPID,szFile);
	}
	else					//	���³�����Ϣ�ļ�
	{
		ret = pcard->update_adf_elf01(bVer,szAPPID,szFile);
	}

	delete pcard;

	return ret;
}

/*21. OBU��ж��־�޸�
	bVer	[in]	OBU��ͬ�汾��
	szAPPID	[in]	OBU��ͬ���к�
	bFlag	[in]	OBU��ж��־
*/
int __stdcall obuUpdateLoadFlag(BYTE bVer,BYTE *szAPPID,BYTE bFlag,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuUpdateLoadFlag)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

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
bool validation(int nlevel,int ncom)
{
	bool bOk = true;
	
	PRINTK("\r\n�������˿�(validation)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	switch(nlevel)
	{
		case 1:
			if(NULL==pcmd[n])
			{
				PRINTK("\nCOM[%d]δ���ӵ�ǰ�ã�",n);
				bOk = false;
				break;
			}
		case 0:
			if(NULL==preader[n])
			{
				PRINTK("\nCOM[%d]������δ�򿪣�",n);
				bOk = false;
				break;
			}
		default:
			break;
	}

	return bOk;
}

/*-------------------------------------------------------------------------
Function:		obuUnlockApplication
Created:		2018-07-28 10:58:19
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				OBU����Ӧ��
-------------------------------------------------------------------------*/
int __stdcall obuUnlockApplication(BYTE bVer,BYTE *szAPPID,int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuUnlockApplication)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(!validation(1,n)) return -1;

	COBUCard *pcard = new COBUCard(preader[n],pcmd[n]);

	ret = pcard->unlockapp(bVer,szAPPID);

	delete pcard;

	return ret;
}







/*-------------------------------------------------------------------------
Function:		setTimeout
Created:		2018-08-27 10:23:14
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall setTimeout(DWORD dwTimeout,int ncom)
{
	PRINTK("\r\n�������˿�(setTimeout)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(NULL==pcmd[n])
	{
		PRINTK("\nCOM[%d]δ���ӵ�ǰ�ã�",n);
		return;
	}
	pcmd[n]->setWaitTimeout(dwTimeout);
	return;
}




/*-------------------------------------------------------------------------
Function:		getTimeout
Created:		2018-08-27 10:23:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
DWORD __stdcall getTimeout(int ncom)
{
	PRINTK("\r\n�������˿�(getTimeout)��%d",ncom);

	int n = ncom%MAX_READER_NUM;

	if(NULL==pcmd[n])
	{
		PRINTK("\nCOM[%d]δ���ӵ�ǰ�ã�",n);
		return 0;
	}
	return pcmd[n]->getWaitTimeout();
}



/*-------------------------------------------------------------------------
Function:		obuOnlineDecodePlate
Created:		2018-08-27 11:33:25
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuOnlineDecodePlate(BYTE bVer,BYTE *szAPPID,
								    BYTE bKeyIndex,
									BYTE bLenIn,BYTE *szEncData,
									BYTE *bLenOut,BYTE *szData,
									int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(obuOnlineDecodePlate)��%d",ncom);

	int icom = ncom%MAX_READER_NUM;
	if(NULL==pcmd[icom]) return -3;

	ret = pcmd[icom]->cmd_1043(bVer,szAPPID,bKeyIndex,bLenIn,szEncData,bLenOut,szData);

	return ret;
}


/*-------------------------------------------------------------------------
Function:		psamOnlineAuth
Created:		2018-08-27 11:33:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall psamOnlineAuth(BYTE *szSAMNo,BYTE *szRnd,
							DWORD dwRoadID,char *strRoadName,
							DWORD dwStationID, char *strStationName,BYTE bStationType,
							BYTE bLaneType,BYTE bLaneID,
							BYTE *bAPDULen,BYTE *szAPDU,
							char *strListNo,
							int ncom)
{
	int ret;
	
	PRINTK("\r\n�������˿�(psamOnlineAuth)��%d",ncom);

	int icom = ncom%MAX_READER_NUM;
	if(NULL==pcmd[icom]) return -3;

	ret = pcmd[icom]->cmd_1044(szSAMNo,szRnd,dwRoadID,strRoadName,dwStationID,strStationName,bStationType,bLaneType,bLaneID,bAPDULen,szAPDU,strListNo);

	return ret;
}






/*-------------------------------------------------------------------------
Function:		psamOnlineSignIn
Created:		2018-08-27 11:33:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall psamOnlineSignIn(BYTE *szSAMNo,BYTE *szTerminalNo,
							DWORD dwRoadID,char *strRoadName,
							DWORD dwStationID, char *strStationName,BYTE bStationType,
							BYTE bLaneType,BYTE bLaneID,
							BYTE *szTerminalTime,
							int ncom)
{
	int ret;
	char strListNo[40];

	PRINTK("\r\n�������˿�(psamOnlineSignIn)��%d",ncom);

	int icom = ncom%MAX_READER_NUM;
	if(NULL==pcmd[icom]) return -3;

	ret = pcmd[icom]->cmd_1046(szSAMNo,szTerminalNo,dwRoadID,strRoadName,dwStationID,strStationName,bStationType,bLaneType,bLaneID,szTerminalTime,strListNo);

	return ret;
}


/*-------------------------------------------------------------------------
Function:		psamOnlineAuthConfirm
Created:		2018-08-27 11:33:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int	__stdcall psamOnlineAuthConfirm(BYTE *szSAMNo,char *strListNo,
							WORD wSW1SW2,BYTE bResult,
							int ncom)
{
	int ret;

	PRINTK("\r\n�������˿�(psamOnlineAuthConfirm)��%d",ncom);

	int icom = ncom%MAX_READER_NUM;
	if(NULL==pcmd[icom]) return -3;

	ret = pcmd[icom]->cmd_1045(szSAMNo,strListNo,wSW1SW2,bResult);

	return ret;
}




/*-------------------------------------------------------------------------
Function:		setBankID
Created:		2018-11-07 09:58:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall setBankID(char *s,int ncom)
{
	if(NULL==s) return;

	PRINTK("\r\n�������˿�(setAgentCode)��%d",ncom);
	
	int icom = ncom%MAX_READER_NUM;

	memset(strBankID,0x00,20);
	if(strlen(s)>19) 
	{
		memcpy(strBankID,s,19);
	}
	else
	{
		strcpy(strBankID,s);
	}

	if(NULL!=pcmd[icom])
	{
		pcmd[icom]->setBankID(strBankID);
	}

	return;

}


/*-------------------------------------------------------------------------
Function:		setAgentCode
Created:		2018-11-07 09:58:57
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall setAgentCode(char *s,int ncom)
{
	if(NULL==s) return;

	PRINTK("\r\n�������˿�(setAgentCode)��%d",ncom);

	int icom = ncom%MAX_READER_NUM;

	memset(strAgentCode,0x00,7);

	if(strlen(s)>6) 
	{
		memcpy(strAgentCode,s,6);
	}
	else
	{
		strcpy(strAgentCode,s);
	}

	if(NULL!=pcmd[icom])
	{
		pcmd[icom]->setAgentCode(strAgentCode);
	}

	return;
}


void	__stdcall	setCardType(int ntype)
{
	gnCardType = ntype;
}