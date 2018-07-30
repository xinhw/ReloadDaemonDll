/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/19 10:35:52

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
		sprintf(strError,"无法加载RSUComm.dll - %08X", dw);
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

	int iDevType = 1;
	if(!memcmp(strAddress,"COM",3))
	{
		iDevType = 0;
	}

	// 打开设备串口
	m_hDevice = lpfn_RSU_Open(iDevType, strAddress, iBaud);
	if(m_hDevice < 0)
	{
		PRINTK("连接设备失败");
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
	
	//	ret = SetMMI_rq(prog_c5.SetMMIMode);
	ret = lpfn_SetMMI_rq(m_hDevice, mode, DID, SetMMIPara, 1000);
	if(ret == 0)
	{
		lpfn_SetMMI_rs(m_hDevice, &DID, &ReturnStatus, 1000);
	}

	ret = lpfn_RSU_Close(m_hDevice);
	Sleep(500);
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

	iResult = lpfn_RSU_INIT_rq(m_hDevice, (char *)szTime, 3, 30, 22, 0, 1000);
	if(iResult!=0)
	{
		PRINTK("\nlpfn_RSU_INIT_rq失败");
		return iResult;
	}

	int iState = 0;

	BYTE btData[128];
	memset(btData,0x00,128);

	iResult =lpfn_RSU_INIT_rs(m_hDevice, &iState, (char *)btData, 1000);
	if( iResult!= 0)
	{
		PRINTK("\nlpfn_RSU_INIT_rs 失败");
		return iResult;
	}

	/*
	iResult = lpfn_PSAM_Reset_rq(m_hDevice, 1, 9600, 2000);
	if(iResult != 0)
	{
		PRINTK("\nlpfn_PSAM_Reset_rq 失败");
		return iResult;
	}

	int iRlen = 0;
	memset(btData,0x00,128);
	iResult = lpfn_PSAM_Reset_rs(m_hDevice, 1, (char *)btData, 1000);
	if( iResult!= 0)
	{
		PRINTK("\nlpfn_PSAM_Reset_rs 失败");
		return iResult;
	}
	*/

	time(&tm);
	iResult = lpfn_INITIALISATION_rq(m_hDevice, "\x08\xFF\xFF\x00", (char *)&tm, 0, 1, "\x01\x1A\x00\x2B\x04", 0, 2000);
	if(iResult != 0)
	{
		PRINTK("\nlpfn_INITIALISATION_rq 失败");
		return iResult;
	}

	int iReturnStatus = 0;
	int iProfile = 0;
	int iApplicationlist = 0;
	BYTE btApplication[128];
	BYTE btObuConfiguration[128];

	memset(btApplication,0x00,128);
	memset(btObuConfiguration,0x00,128);

	iResult = lpfn_INITIALISATION_rs(m_hDevice, &iReturnStatus, &iProfile, &iApplicationlist, (char *)btApplication, (char *)btObuConfiguration, 2000);
	if(iResult != 0)
	{
		PRINTK("\n无法找到OBU");
		return iResult;
	}

	return 0;
}



// 向卡机发送命令
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
	// 将字符串转换成字节序列
	pBuffer[0] = strlen(strCmd)/2;
	CMisc::StringToByte(strCmd, pBuffer+1);

	// 发送COS命令
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

	// 将返回的字节序列转换成字符串
	CMisc::ByteToString(&btData[1], btData[0],strResult);

#ifdef DEBUG_PRINT
	PRINTK("\nOBU-RSP:%s",strResult);
#endif

	if(strlen(strResult) < 4)
	{
		sprintf(strResult,"%x", &iResult);
		return iResult;
	}

	char strState[5];
	
	memset(strState,0x00,5);
	memcpy(strState,strResult+strlen(strResult) - 4, 4);
	strResult[strlen(strResult)-4]=0x00;
	
	sprintf(strState, "%x", &iResult);

	return iResult;
}

// 向卡机发送命令
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
	BYTE *pBuffer = new BYTE[strlen(strCmd)/2 + 1];

	memset(pBuffer, 0, strlen(strCmd)/2);

	// 将字符串转换成字节序列
	CMisc::StringToByte(strCmd, &pBuffer[1]);
	pBuffer[0] = strlen(strCmd) / 2;

	// 发送COS命令
	unsigned int iLen = 0;
	clock_t tStart = clock();
	if(lpfn_PSAM_CHANNEL_rq(m_hDevice, 2, 1, (char *)pBuffer, 2000) != SUCCESS)
	{
		return FALSE;
	}

	int iDID = 0;
	int iChannelID = 0;
	int iAPDULIST = 0;
	BYTE btData[512];
	iResult = lpfn_PSAM_CHANNEL_rs(m_hDevice, 2, &iAPDULIST, (char *)btData, 2000);
	if(iResult != SUCCESS)
	{
		return FALSE;
	}

	clock_t tEnd = clock();

	// 将返回的字节序列转换成字符串
	CMisc::ByteToString(&btData[1], btData[0] + 2,strResult);

	// 记录命令日志
	// 时间总计
	AddTimeSum(((double)(tEnd - tStart) / CLOCKS_PER_SEC) * 1000);

	// 记录命令日志
	char strTime[20];
	sprintf(strTime,"FM>>%s (%0.0f ms)", strCmd, ((double)(tEnd - tStart) / CLOCKS_PER_SEC) * 1000);
	PRINTK(strTime);
	PRINTK(strResult);

	delete []pBuffer;

	if(strlen(strResult) < 4)
	{
		sprintf(strResult, "%x", &iResult);
		return iResult;
	}

	char strState[5];
	
	memset(strState,0x00,5);
	memcpy(strState,strResult+strlen(strResult) - 4, 4);

	strResult[strlen(strResult)-4]=0x00;
	sprintf(strState, "%x", &iResult);

	return iResult;
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
	