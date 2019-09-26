/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/19 10:35:52

    Reversion:
        
-------------------------------------------------------------------------*/

#include "NXRsuReader.h"

/*-------------------------------------------------------------------------
Function:		CNXRsuReader.CNXRsuReader
Created:		2018-07-19 10:36:26
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CNXRsuReader::CNXRsuReader(void)
{
	m_hDll = NULL;
	m_hDevice = NULL;

	m_bPSAMNode = 0x01;
	memset(m_szApplication,0x00,16);
}

/*-------------------------------------------------------------------------
Function:		CNXRsuReader.~CNXRsuReader
Created:		2018-07-19 10:36:28
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CNXRsuReader::~CNXRsuReader(void)
{
}




/*-------------------------------------------------------------------------
Function:		CNXRsuReader.Open
Created:		2018-07-19 10:36:31
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CNXRsuReader::Open(char *strAddress, unsigned int iBaud)
{
	m_hDll = LoadLibrary("RSUComm.dll");
	if(m_hDll == NULL)
	{
		DWORD dw = GetLastError();
		char strError[32];
		sprintf(strError,"�޷������򼯶�������̬��RSUComm.dll - %08X", dw);
		PRINTK(strError);
		return 0x1A13;
	}

	m_ulCardNum = 0;

	lpfn_RSU_Open = (LPFN_RSU_Open)GetProcAddress(m_hDll, "RSU_Open");
	lpfn_RSU_Close = (LPFN_RSU_Close)GetProcAddress(m_hDll, "RSU_Close");
	lpfn_RSU_INIT_rq = (LPFN_RSU_INIT_rq)GetProcAddress(m_hDll, "RSU_INIT_rq");
	lpfn_RSU_INIT_rs = (LPFN_RSU_INIT_rs)GetProcAddress(m_hDll, "RSU_INIT_rs");
	lpfn_PSAM_Reset_rq = (LPFN_PSAM_Reset_rq)GetProcAddress(m_hDll, "PSAM_Reset_rq");
	lpfn_PSAM_Reset_rs = (LPFN_PSAM_Reset_rs)GetProcAddress(m_hDll, "PSAM_Reset_rs");
	lpfn_PSAM_CHANNEL_rq = (LPFN_PSAM_CHANNEL_rq)GetProcAddress(m_hDll, "PSAM_CHANNEL_rq");
	lpfn_PSAM_CHANNEL_rs = (LPFN_PSAM_CHANNEL_rs)GetProcAddress(m_hDll, "PSAM_CHANNEL_rs");
	lpfn_INITIALISATION_rq = (LPFN_INITIALISATION_rq)GetProcAddress(m_hDll, "INITIALISATION_rq");
	lpfn_INITIALISATION_rs = (LPFN_INITIALISATION_rs)GetProcAddress(m_hDll, "INITIALISATION_rs");
	lpfn_TransferChannel_rq = (LPFN_TransferChannel_rq)GetProcAddress(m_hDll, "TransferChannel_rq");
	lpfn_TransferChannel_rs = (LPFN_TransferChannel_rs)GetProcAddress(m_hDll, "TransferChannel_rs");
	lpfn_SetMMI_rq = (LPFN_SetMMI_rq)GetProcAddress(m_hDll, "SetMMI_rq");
	lpfn_SetMMI_rs = (LPFN_SetMMI_rs)GetProcAddress(m_hDll, "SetMMI_rs");
	lpfn_GetSecure_rq = (LPFN_GetSecure_rq)GetProcAddress(m_hDll, "GetSecure_rq");
	lpfn_GetSecure_rs = (LPFN_GetSecure_rs)GetProcAddress(m_hDll,"GetSecure_rs");


	int iDevType = 1;
	if(!memcmp(strAddress,"COM",3))
	{
		iDevType = 0;
	}

	// ���豸����
	m_hDevice = lpfn_RSU_Open(iDevType, strAddress, iBaud);
	if(m_hDevice < 0)
	{
		PRINTK("�����豸ʧ��");
		return -1;
	}


	return 0;
}

/*-------------------------------------------------------------------------
Function:		CNXRsuReader.Close
Created:		2018-07-19 10:36:35
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void CNXRsuReader::Close()
{
	int ret;
	int mode = 1;
	int DID =1;
	int SetMMIPara = 0;
	int ReturnStatus = 0;
	

	if(NULL==lpfn_SetMMI_rq) return;

	ret = lpfn_SetMMI_rq(m_hDevice, mode, DID, SetMMIPara, 1000);
	if(ret == 0)
	{
		lpfn_SetMMI_rs(m_hDevice, &DID, &ReturnStatus, 1000);
	}

	ret = lpfn_RSU_Close(m_hDevice);
	Sleep(1);
	FreeLibrary(m_hDll);
	m_hDll = NULL;
	m_hDevice = NULL;
}

/*-------------------------------------------------------------------------
Function:		CNXRsuReader.Initialize
Created:		2018-07-19 10:36:52
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CNXRsuReader::Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult)
{
	time_t tm;
	time(&tm);
	BYTE szTime[4];

	int iResult = 0;

	bATSLen = 0;

	memset(szTime,0x00,4);
	CMisc::Int2Bytes(tm,szTime);

	iResult = iResult = lpfn_RSU_INIT_rq(m_hDevice, (char *)szTime, 3, 30, 22, 0, 1000);
	if(iResult!=0)
	{
		PRINTK("\nlpfn_RSU_INIT_rqʧ��");
		return iResult;
	}

	int iState = 0;
	BYTE btData[128];
	memset(btData,0x00,128);

	iResult =lpfn_RSU_INIT_rs(m_hDevice, &iState, (char *)btData, 1000);
	if( iResult!= 0)
	{
		PRINTK("\nlpfn_RSU_INIT_rs ʧ��");
		return iResult;
	}

	time(&tm);
	iResult = lpfn_INITIALISATION_rq(m_hDevice, "\x08\xFF\xFF\x00", (char *)&tm, 0, 1, "\x01\x1A\x00\x2B\x04", 0, 2000);
	if(iResult != 0)
	{
		PRINTK("\nlpfn_INITIALISATION_rq ʧ��");
		return iResult;
	}

	int iReturnStatus = 0;
	int iProfile = 0;
	int iApplicationlist = 0;
	BYTE btApplication[256];
	BYTE btObuConfiguration[128];

	memset(btApplication,0x00,256);
	memset(btObuConfiguration,0x00,128);

	iResult = lpfn_INITIALISATION_rs(m_hDevice, &iReturnStatus, &iProfile, &iApplicationlist, (char *)btApplication, (char *)btObuConfiguration, 2000);
	if(iResult != 0)
	{
		PRINTK("\n�޷��ҵ�OBU");
		return iResult;
	}


	//	��14�ֽڿ�ʼ��ȡ8�ֽڣ��ӵ�4�ֽڿ�ʼ��ȡ4�ֽڣ��ӵ�4�ֽڿ�ʼ��ȡ4�ֽ�
	memcpy(m_szApplication,btApplication+12,8);
	memcpy(m_szApplication+8,btApplication+2,4);
	memcpy(m_szApplication+12,btApplication+2,4);


	int i;
	PRINTK("\nbtApplication:");
	for(i=0;i<32;i++) PRINTK("%02X ",(BYTE)btApplication[i]);

	PRINTK("\nOBU APPLICATION:");
	for(i=0;i<16;i++) PRINTK("%02X",(BYTE)m_szApplication[i]);

	return 0;
}



// �򿨻���������
/*-------------------------------------------------------------------------
Function:		CNXRsuReader.RunCmd
Created:		2018-07-19 10:36:58
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CNXRsuReader::RunCmd(char *strCmd, char *strResult)
{
	int iResult = 0;
	BYTE pBuffer[257];

	memset(pBuffer, 0, 257);

#ifdef DEBUG_PRINT
	PRINTK("\nOBU-CMD:%s",strCmd);
#endif
	// ���ַ���ת�����ֽ�����
	pBuffer[0] = strlen(strCmd)/2;
	CMisc::StringToByte(strCmd, pBuffer+1);

	// ����COS����
	iResult = lpfn_TransferChannel_rq(m_hDevice, 1, 1, 2, 1, (char *)pBuffer, 1000);
	if( iResult!= SUCCESS)
	{
		return iResult;
	}

	int iDID = 0;
	int iChannelID = 0;
	int iAPDULIST = 0;
	BYTE btData[512];
	int iReturnStatus;

	memset(btData,0x00,512);
	iResult = lpfn_TransferChannel_rs(m_hDevice, &iDID, &iChannelID, &iAPDULIST, (char *)btData, &iReturnStatus, 1000);
	if(iResult != SUCCESS)
	{
		#ifdef DEBUG_PRINT
			PRINTK("\nOBU-RSP: FAILURE WITH ERROR %d",strResult);
		#endif
		return iResult;
	}

	// �����ص��ֽ�����ת�����ַ���
	CMisc::ByteToString(&btData[1], btData[0],strResult);

#ifdef DEBUG_PRINT
	PRINTK("\nOBU-RSP:%s",strResult);
#endif

	if(strlen(strResult) < 4)
	{
		sprintf(strResult,"%x", &iResult);
		return 0x6710;
	}

	char strState[5];
	
	memset(strState,0x00,5);
	memcpy(strState,strResult+strlen(strResult) - 4, 4);
	strResult[strlen(strResult)-4]=0x00;

	iResult = CMisc::ascToUC(strState[0]);
	iResult = iResult*0x10 + CMisc::ascToUC(strState[1]);
	iResult = iResult*0x10 + CMisc::ascToUC(strState[2]);
	iResult = iResult*0x10 + CMisc::ascToUC(strState[3]);

	if(iResult==0x9000) iResult = 0;

	return iResult;
}


// �򿨻���������
/*-------------------------------------------------------------------------
Function:		CNXRsuReader.PSAM_RunCmd
Created:		2018-07-19 10:37:03
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CNXRsuReader::PSAM_RunCmd(char *strCmd, char *strResult)
{
	int iResult = 0;
	int iDID = 0;
	int iChannelID = 0;
	int iAPDULIST = 0;
	BYTE btData[512];
	// ����COS����
	unsigned int iLen = 0;

	BYTE *pBuffer = new BYTE[strlen(strCmd)/2 + 1];

	memset(pBuffer, 0, strlen(strCmd)/2);

	// ���ַ���ת�����ֽ�����
	CMisc::StringToByte(strCmd, &pBuffer[1]);
	pBuffer[0] = strlen(strCmd) / 2;


#ifdef DEBUG_PRINT
	PRINTK("\nPSAM CMD:%s",strCmd);
#endif

	iResult = lpfn_PSAM_CHANNEL_rq(m_hDevice, m_bPSAMNode, 1, (char *)pBuffer, 2000);
	if(iResult != SUCCESS)
	{
		#ifdef DEBUG_PRINT
			PRINTK("\nPSAM: CMD FAILURE WITH ERROR %d",strResult);
		#endif
		delete []pBuffer;
		return iResult;
	}

	memset(btData,0x00,512);
	iResult = lpfn_PSAM_CHANNEL_rs(m_hDevice, m_bPSAMNode, &iAPDULIST, (char *)btData, 2000);
	if(iResult != SUCCESS)
	{
		#ifdef DEBUG_PRINT
			PRINTK("\nPSAM: RSP FAILURE WITH ERROR %d",strResult);
		#endif
		delete []pBuffer;
		return iResult;
	}

	// �����ص��ֽ�����ת�����ַ���
	CMisc::ByteToString(btData+1, btData[0],strResult);

	delete []pBuffer;

	iResult = btData[btData[0]+1];
	iResult = iResult *0x100 + btData[btData[0]+2]; 

#ifdef DEBUG_PRINT
	PRINTK("\nPSAM RSP:%s %04X",strResult,iResult);
#endif

	if(iResult==0x9000) return 0;

	return iResult;
}


/*-------------------------------------------------------------------------
Function:		CNXRsuReader.PSAM_RunCmd
Created:		2018-07-19 10:37:03
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CNXRsuReader::PSAM_Atr(BYTE bNode,BYTE &brLen,char *strATR)
{
	int iResult = 0;
	BYTE btData[128];

	time_t tm;
	BYTE szTime[4];

	m_bPSAMNode = bNode;

	memset(btData,0x00,128);

	time(&tm);
	memset(szTime,0x00,4);
	CMisc::Int2Bytes(tm,szTime);

	iResult = lpfn_PSAM_Reset_rq(m_hDevice, m_bPSAMNode, 9600, 2000);
	if(iResult != 0)
	{
		PRINTK("\nlpfn_PSAM_Reset_rq ʧ��");
		return iResult;
	}

	int iRlen = 0;
	memset(btData,0x00,128);
	iResult = lpfn_PSAM_Reset_rs(m_hDevice, m_bPSAMNode, (char *)btData, 2000);
	if( iResult!= 0)
	{
		PRINTK("\nlpfn_PSAM_Reset_rs ʧ��");
		return iResult;
	}
	
	return 0;
}


/*-------------------------------------------------------------------------
Function:		CNXRsuReader.Halt
Created:		2018-07-19 10:37:07
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CNXRsuReader::Halt()
{
	return 0;
}

/*-------------------------------------------------------------------------
Function:		CNXRsuReader.SecureRead
Created:		2018-07-19 10:37:03
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
				bKeyindex
-------------------------------------------------------------------------*/
UINT CNXRsuReader::SecureRead(BYTE bKeyIndex,BYTE bFileID,BYTE bOffset,BYTE bLength,
							BYTE &bRetFileLen,char *strResp)
{

	int ret;

    int accessCredentialsOp = 0;               //�Ƿ������֤��,0��ʾ��������֤�룬1��ʾ����;ȡֵ��0
    int mode = 1;                              //ȷ��ģʽ��1����Ӧ��,0������Ӧ��;ȡֵ��1
    int did = 1;                               //Ҫ��ȡ��OBUӦ�úţ�ETCӦ��Ϊ1����ʶվӦ��Ϊ2,���е�·�շ�Ϊ3,4-10����,ȡֵ��1
    BYTE accessCredentials[9];				   //OBU��֤�룬8�ֽڣ�����accessCredentialsOp����RSU����������֡���Ƿ��и���
    int keyIdForEncryptOp = 1;                 //�Ƿ����keyIdForEncrypt��0�����ڣ�1����; ȡֵ��1

    BYTE randRsu[9];						   //RSU�����������
    //int keyIdForAuthen = 0;                  //��Կ������
    int keyIdForEncrypt = 0;                   //������Կ������

	memset(accessCredentials,0x00,9);
	memset(randRsu,0x00,9);

	memcpy(randRsu,"\x79\xCA\x5F\xB2\xA3\xCA\xFB\xF0",8);
	
	/*
	int i;
	PRINTK("\n�Ƿ������֤��:%02X",accessCredentialsOp);
	PRINTK("\nȷ��ģʽ:%02X",mode);
	PRINTK("\nҪ��ȡ��OBUӦ�ú�:%02X",did);
	PRINTK("\nOBU��֤��:");
	for(i=0;i<8;i++) PRINTK("%02X",accessCredentials[i]);
	PRINTK("\n�Ƿ����keyIdForEncrypt��:%02X",keyIdForEncryptOp);
	PRINTK("\nOBU�ļ��ţ�%02X",bFileID);
	PRINTK("\n�ļ�ƫ������%02X",bOffset);
	PRINTK("\n�ļ����ȣ�%02X",bLength);
	PRINTK("\nRSU�����������:");
	for(i=0;i<8;i++) PRINTK("%02X",randRsu[i]);
	PRINTK("\n��Կ������:%02X",bKeyIndex);
	PRINTK("\n������Կ������:%02X",keyIdForEncrypt);
	PRINTK("\n�Ƿ����keyIdForEncryptOp:%02X",keyIdForEncryptOp);
	*/

	ret = lpfn_GetSecure_rq(m_hDevice,accessCredentialsOp,mode,did,(char *)accessCredentials,
							keyIdForEncryptOp, 
							bFileID,bOffset, bLength,
							(char *)randRsu, bKeyIndex, keyIdForEncrypt,
							2000);
	if( ret!= 0)
	{
		PRINTK("\nlpfn_GetSecure_rq ʧ��:%d",ret);
		return ret;
	}

    int obuFileId = 1;                         //OBU�ļ���
    int obuFileOffset = 0;                     //OBU�ļ�ƫ����
    int obuFileLength = 16;                    //OBU�ļ�����
	BYTE replyData[201];
	BYTE authenticator[9];
	int obuDealState = 0;               //OBU����״̬

	memset(authenticator,0x00,9);
	memset(replyData,0x00,201);

	ret = lpfn_GetSecure_rs(m_hDevice,&did,&obuFileId,&obuFileLength, (char *)replyData, (char *)authenticator,&obuDealState, 2000);
	if( ret!= 0)
	{
		PRINTK("\nlpfn_GetSecure_rs ʧ��:%d",ret);
		return ret;
	}
	PRINTK("\nFileID=%02X\tFile Length=%02x",obuFileId, obuFileLength);

	bRetFileLen = (BYTE)obuFileLength;
	strResp[0]=0x00;
	CMisc::ByteToString(replyData,bRetFileLen,strResp);

	PRINTK("\nREPLYDATA:%s",strResp);

	return 0;
}



UINT CNXRsuReader::Beep(BYTE bAct)
{
	PRINTK("\n�򼯶�������ʱδʵ��Beep����");
	return -1;
}