/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/19 10:35:00

    Reversion:
        
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
Function:		CAISINOReader.CAISINOReader
Created:		2018-07-19 10:35:03
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CAISINOReader::CAISINOReader()
{
	m_hDevice=0;
	
	lp_jt_openreader=NULL;
	lp_jt_closereader=NULL;
	lp_jt_readerversion=NULL;
	lp_jt_getlasterror=NULL;
	lp_jt_opencard=NULL;
	lp_jt_closecard=NULL;
	lp_jt_audiocontrol=NULL;
	lp_jt_cpucommand=NULL;
}


/*-------------------------------------------------------------------------
Function:		CAISINOReader.~CAISINOReader
Created:		2018-07-19 10:35:06
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CAISINOReader::~CAISINOReader()
{
	Close();
	if(m_hDll!=NULL) FreeLibrary(m_hDll);
}


int CAISINOReader::initDll()
{
	if(m_hDll!=NULL) FreeLibrary(m_hDll);

	m_hDll = LoadLibrary("API_JKM115.dll");
	if(m_hDll == NULL)
	{
		DWORD dw = GetLastError();
		PRINTK("无法加载API_JKM115.dll - %08X", dw);
		return 0x1A13;
	}

	lp_jt_openreader = (LPFN_JT_OpenReader)GetProcAddress(m_hDll, "JT_OpenReader");
	lp_jt_closereader = (LPFN_JT_CloseReader)GetProcAddress(m_hDll, "JT_CloseReader");
	lp_jt_readerversion = (LPFN_JT_ReaderVersion)GetProcAddress(m_hDll, "JT_ReaderVersion");
	//lp_jt_getlasterror = (LPFN_JT_GetLastError)GetProcAddress(m_hDll, "JT_GetLastError");
	lp_jt_opencard = (LPFN_JT_OpenCard)GetProcAddress(m_hDll, "JT_OpenCard");
	lp_jt_closecard = (LPFN_JT_CloseCard)GetProcAddress(m_hDll, "JT_CloseCard");
	lp_jt_audiocontrol = (LPFN_JT_AudioControl)GetProcAddress(m_hDll, "JT_AudioControl");
	lp_jt_cpucommand = (LPFN_JT_CPUCCommand)GetProcAddress(m_hDll, "JT_CPUCommand");


	return 0;
}
/*-------------------------------------------------------------------------
Function:		CAISINOReader.Open
Created:		2018-07-19 10:35:09
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CAISINOReader::Open(char *strAddress, unsigned int iBaud)
{
	char Paras[32];
	int ret;
	int m_icom;

	ret = initDll();
	if(ret) return ret;

	m_icom = atoi(strAddress+3);

	memset(Paras,0x00,32);
	sprintf(Paras,"%d,n,8,1",iBaud);

	m_hDevice = lp_jt_openreader(m_icom,Paras);

	if(m_hDevice<=0)
	{
		PRINTK("\n打开(%s)上的航天金卡读卡器失败:%d",Paras,m_hDevice);
		return 0x1A14;
	}
	
	return 0;
}


/*-------------------------------------------------------------------------
Function:		CAISINOReader.Close
Created:		2018-07-19 10:35:12
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void CAISINOReader::Close()
{
	if(lp_jt_closereader!=NULL)
	{
		lp_jt_closereader(m_hDevice);
	}

	m_hDevice = 0;
	return;
}


/*-------------------------------------------------------------------------
Function:		CAISINOReader.Initialize
Created:		2018-07-19 10:35:17
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CAISINOReader::Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult)
{
	int ret;
	BYTE pszBuf[128];
	int ilen;

	if(lp_jt_opencard==NULL) return -1;

	memset(pszBuf,0x00,128);
	ilen = 0;

	ret = lp_jt_opencard(m_hDevice,pszBuf);
	if(0==ret)
	{
		memcpy(strsno,pszBuf,4);
		return 0;
	}
	return -1;
}

/*-------------------------------------------------------------------------
Function:		CAISINOReader.RunCmd
Created:		2018-07-19 10:35:20
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CAISINOReader::RunCmd(char *strCmd, char *strResult)
{
	int ret;
	BYTE pszCmd[256],pszBuf[256],buf[5];
	int ilen,bCmdLen;
	BYTE sw1,sw2;

	if(lp_jt_cpucommand==NULL) return -1;


	memset(pszCmd,0x00,256);
#ifdef DEBUG_PRINT
	BYTE i;
	PRINTK("\nCMD:%s",strCmd);
#endif

	bCmdLen = strlen(strCmd)/2;
	CMisc::StringToByte(strCmd, pszCmd);

	ilen = 0;
	memset(pszBuf,0x00,256);
	ret = lp_jt_cpucommand(m_hDevice,pszCmd,bCmdLen,pszBuf,&ilen);
	if(ret) return ret;

	sw1 = pszBuf[0];
	sw2 = pszBuf[1];
	if(sw1==0x61)
	{
		memset(buf,0x00,5);
		buf[1] = 0xc0;buf[4]=sw2;

		ilen = 0;
		memset(pszBuf,0x00,256);

		ret = lp_jt_cpucommand(m_hDevice,buf,5,pszBuf,&ilen);
		if(ret) return ret;
	}

	CMisc::ByteToString(pszBuf+2,ilen-2,strResult);

	ret = sw1;
	ret = ret*0x100 + sw2;

#ifdef DEBUG_PRINT
	PRINTK("\nRSP:");
	for(i=0;i<(ilen-2);i++) PRINTK("%02X",pszBuf[2+i]);
	PRINTK("%04X",ret);
#endif


	if(ret==0x9000) return 0;

	return ret;
}

/*-------------------------------------------------------------------------
Function:		CAISINOReader.Halt
Created:		2018-07-19 10:35:24
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CAISINOReader::Halt()
{
	if(lp_jt_closecard==NULL) return -1;

	lp_jt_closecard(m_hDevice);
	return 0;
}


UINT CAISINOReader::PSAM_RunCmd(char *strCmd, char *strResult)
{
	PRINTK("\n航天金卡读卡器暂时未实现PSAM指令函数");
	return -1;
}

UINT CAISINOReader::PSAM_Atr(BYTE bNode,BYTE &brLen,char *strATR)
{
	PRINTK("\n航天金卡读卡器暂时未实现PSAM复位函数");
	return -1;
}

UINT CAISINOReader::SecureRead(BYTE bKeyIndex,BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE &bRetFileLen,char *strResp)
{
	PRINTK("\n航天金卡读卡器暂时未实现SecureRead函数");
	return -1;
}