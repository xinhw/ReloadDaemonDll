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

/*	�����ն˻����*/
/*-------------------------------------------------------------------------
Function:		setDeviceNo
Created:		2018-07-16 17:04:14
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall setDeviceNo(BYTE *szNo)
{
	if(NULL!=szNo) memcpy(gszDeviceNo,szNo,6);
	return;
}



/*	��ȡ�ն˻����*/
/*-------------------------------------------------------------------------
Function:		getDeviceNo
Created:		2018-07-16 17:04:17
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void __stdcall getDeviceNo(BYTE *szNo)
{
	if(NULL!=szNo) memcpy(szNo,gszDeviceNo,6);
	return;
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
int __stdcall connectOKS(char *strip,WORD wport)
{
	int			ret;
	WSADATA		wsaData;  	

	//	WinSock2��ʼ��
	WORD wSockVersion = MAKEWORD(2,1);
	ret = WSAStartup(wSockVersion,&wsaData);
	if(ret) 
	{
		PRINTK("\nWIN SOCKET2��ʼ��ʧ�ܣ�");
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
		PRINTK("\ninit_socketʧ��:%d",ret);
		return ret;
	}

	ret = ptransfer->connect_server(wport,strip);
	if(ret)
	{
		PRINTK("\nconnect_server���ӷ�����%s:%dʧ��:%d",strip,wport,ret);
		delete ptransfer;
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

/*2. �رյ�������Կ����ǰ�õ�����*/
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

	if(NULL!=preader)
	{
		preader->Close();
		delete preader;
	}
	
	//	CPU��������������𿨣�
	if(READER_TYPE_CPU_CARD==nType)
	{
		preader = new CAISINOReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader->Open(strcom,nbaud);
		return ret;
	}

	//	OBU���������򼯶�������
	if(READER_TYPE_OBU==nType)
	{
		preader = new CNXRsuReader();

		memset(strcom,0x00,32);
		sprintf(strcom,"COM%d",ncom);
		ret = preader->Open(strcom,nbaud);
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
/*				CPU����																   */
/***************************************************************************************/
/*6. CPU����λ
	szSNO	[in]	��Ψһ��
	bATSLen	[in]	��ƬATS���صĳ���
	szATS	[in]	��ƬATS��Ϣ
*/
int __stdcall cpuATS(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS)
{
	int ret;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->rats(szSNO,bATSLen,szATS);

	delete pcard;

	return ret;
}


/*7. ��CPU���ļ�
	wFileID	[in]	�ļ���SFI
	bLen	[out]	�ļ�����
	szFile	[out]	�ļ�����
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

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->readCard(elf15,elf16,dwRemain);

	delete pcard;

	return ret;
}

/*
	���Զ�ȡ��0018,0017,0019�ļ�
*/
int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec)
{
	int ret;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->readRecord(bFileID,bNo,bLen,szRec);

	delete pcard;

	return ret;

}

/*8. CPU��һ��
	szFile0015	[in]	0015�ļ�����
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

	CTYCPUCard *pcard = new CTYCPUCard(preader,pcmd);

	ret = pcard->init(szFile0015);

	delete pcard;

	return ret;

}


/*9. ���³ֿ��˻��������ļ�
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
int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile)
{
	return 0;
}


/*10. ���¿����л��������ļ�
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
int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile)
{
	return 0;
}



/*11. ���¿�Ƭ��Ч��
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
int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate)
{
	return 0;
}

/*12. Ȧ��
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
int __stdcall cpuCredit(BYTE bCardVer,BYTE *szAPPID,DWORD dwAmount,BYTE *szDateTime,
						WORD &wSeqNo,BYTE *szTAC)
{
	int ret;

	CCPUCardBase *pcard = new CCPUCardBase(preader,pcmd);

	ret = pcard->credit(bCardVer,szAPPID,gszDeviceNo,dwAmount,wSeqNo,szDateTime,szTAC);

	delete pcard;

	return ret;
}

/*13. ����
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
int __stdcall cpuPurchase(BYTE bCardVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,
						WORD &wSeqNo,BYTE *szTAC)
{
	return 0;
}

/*13. CPU����֤TAC
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
int __stdcall cpuVerifyTAC(BYTE bCardVer,BYTE *szAPPID,
						   DWORD dwAmount,BYTE bTransFlag,DWORD dwAuditNo,BYTE *szDateTime,
						   BYTE *szTAC)
{
	return 0;
}


/*
14. CPU����װ��Կ
	bVer		[in]	��Ƭ�汾��
	szAPPID		[in]	��ƬӦ�����к�
*/
/*-------------------------------------------------------------------------
Function:		cpuReloadPIN
Created:		2018-07-16 17:05:07
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall cpuReloadPIN(BYTE bCardVer,BYTE *szAPPID)
{
	return 0;
}


/***************************************************************************************/
/*				OBU����	    														   */
/***************************************************************************************/
/*6. OBU��λ
	bATRLen	[in]	OBU��ATR���صĳ���
	szATR	[in]	OBU��ATR��Ϣ
*/
/*-------------------------------------------------------------------------
Function:		obuATR
Created:		2018-07-16 17:05:10
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall obuATR(BYTE &bATRLen,BYTE *szATR)
{
	return 0;
}

/*8. OBUһ��
	szEF01	[in]	������Ϣ�ļ�
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
	return 0;
}

/*8. OBU��ȡ��Ϣ
	bVer	[in]	OBU��ͬ�汾��
	szAPPID	[in]	OBU��ͬ���к�
	wFileID	[in]	�ļ�ID
	bLen	[out]	�ļ�����
	szFile	[out]	�ļ���Ϣ
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

	COBUCard *pcard = new COBUCard(preader,pcmd);

	ret = pcard->read_obu(elf01_mk,elf01_adf01);

	delete pcard;

	return ret;

}


/*8. OBU�����ļ�
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
int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bLen,BYTE *szFile)
{
	return 0;
}


