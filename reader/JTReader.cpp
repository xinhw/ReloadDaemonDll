/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/31 15:59:18

    Reversion:
        
-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------
Function:		CJTReader.CJTReader
Created:		2018-07-31 15:59:20
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CJTReader::CJTReader()
{
	m_icom= -1;

}



/*-------------------------------------------------------------------------
Function:		CJTReader.~CJTReader
Created:		2018-07-31 15:59:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CJTReader::~CJTReader()
{
	Close();
	if(m_hDll!=NULL) FreeLibrary(m_hDll);
}



/*-------------------------------------------------------------------------
Function:		CJTReader.initDll
Created:		2018-07-31 15:59:27
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CJTReader::initDll()
{
	if(m_hDll!=NULL) FreeLibrary(m_hDll);

	m_hDll = LoadLibrary("CDRDLL.dll");
	if(m_hDll == NULL)
	{
		DWORD dw = GetLastError();
		PRINTK("无法加载CDRDLL.dll - %08X", dw);
		return 0x1A13;
	}

	lp_jt_openreader = (LPFN_JT_OpenReader)GetProcAddress(m_hDll, "JT_OpenReader");
	lp_jt_closereader = (LPFN_JT_CloseReader)GetProcAddress(m_hDll, "JT_CloseReader");
	lp_jt_opencard = (LPFN_JT_OpenCard)GetProcAddress(m_hDll, "JT_OpenCard");
	lp_jt_closecard = (LPFN_JT_CloseCard)GetProcAddress(m_hDll, "JT_CloseCard");
	lp_jt_cpucommand = (LPFN_JT_CPUCCommand)GetProcAddress(m_hDll, "JT_CPUCCommand");


	return 0;
}

/*-------------------------------------------------------------------------
Function:		CJTReader.Open
Created:		2018-07-31 15:59:30
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CJTReader::Open(char *strAddress, unsigned int iBaud)
{
	int ret;

	ret = initDll();
	if(ret) return ret;

	if(NULL==lp_jt_openreader) return -1;
	

	m_icom = atoi(strAddress+3);

	//函数返回值定义： 	0：成功	其他：失败
	ret = lp_jt_openreader(m_icom,"EMP5010.ini");
	if(ret)
	{
		m_icom = 0;
		PRINTK("\n打开上的深圳雄帝的读卡器[%d]失败:%d",strAddress,ret);
		return 0x1A14;
	}
	

	return 0;
}

/*-------------------------------------------------------------------------
Function:		CJTReader.Close
Created:		2018-07-31 15:59:34
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void CJTReader::Close()
{
	if(lp_jt_closereader!=NULL)
	{
		lp_jt_closereader(m_icom);
	}

	m_icom = -1;
}


/*-------------------------------------------------------------------------
Function:		CJTReader.Initialize
Created:		2018-07-31 15:59:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CJTReader::Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult)
{
	int ret;
	BYTE szATR[128];
	int nlen;

	nlen =0;
	memset(szATR,0x00,128);

	if(lp_jt_opencard==NULL) return -1;
	if(NULL!=lp_jt_closecard) lp_jt_closecard(m_icom);

	if(m_icom<0) 
	{
		PRINTK("\n无效的COM口：%d",m_icom);
		return -1;
	}

	nlen = 128;
	ret = lp_jt_opencard(m_icom,szATR,&nlen);
	if(ret<0) 
	{
		PRINTK("\njt_opencard失败:%d",ret);
		return ret;
	}
	// 	0：无卡
	// 	1：卡片类型为S50卡块格式
	// 	2：卡片类型为S50卡MAD 格式
	// 	3：卡片为PRO 卡
	// 	4：卡片类型为S70卡块格式
	// 	5：卡片类型为S70卡MAD格式
	// 	其他正值：预留的卡片类型定义值
	// 	其他负值：失败

	PRINTK("\njt_opencard返回:%d",ret);
	if(3==ret)
	{
		memcpy(strsno,szATR,4);
		memcpy(strResult,szATR+4,nlen-4);
		bATSLen = nlen -4;

		ret = 0;
	}
	return ret;
}

/*-------------------------------------------------------------------------
Function:		CJTReader.RunCmd
Created:		2018-07-31 15:59:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CJTReader::RunCmd(char *strCmd, char *strResult)
{
	int ret;
	BYTE pszCmd[256],pszBuf[256],buf[5];
	int ilen,bCmdLen;
	BYTE sw1,sw2;

	if(lp_jt_cpucommand==NULL)
	{
		PRINTK("\n无效的JT_CPUCCommand函数");
		return -1;
	}
	if(m_icom<0) return -1;

	memset(pszCmd,0x00,256);
#ifdef DEBUG_PRINT
	BYTE i;
	PRINTK("\nCMD:%s",strCmd);
#endif

	bCmdLen = strlen(strCmd)/2;
	CMisc::StringToByte(strCmd, pszCmd);

	ilen = 0;
	memset(pszBuf,0x00,256);
	ilen = 256;
	ret = lp_jt_cpucommand(m_icom,pszCmd,bCmdLen,pszBuf,&ilen);
	if(ret)
	{
		PRINTK("\nJT_CPUCCommand函数失败:%d",ret);
		return ret;
	}
	sw1 = pszBuf[0];
	sw2 = pszBuf[1];
	if(sw1==0x61)
	{
		memset(buf,0x00,5);
		buf[1] = 0xc0;buf[4]=sw2;

		ilen = 0;
		memset(pszBuf,0x00,256);

		ret = lp_jt_cpucommand(m_icom,buf,5,pszBuf,&ilen);
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
Function:		CJTReader.Halt
Created:		2018-07-31 15:59:46
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CJTReader::Halt()
{
	if(lp_jt_closecard==NULL) return -1;

	if(m_icom>=0) lp_jt_closecard(m_icom);

	return 0;
}

UINT CJTReader::PSAM_RunCmd(char *strCmd, char *strResult)
{
	PRINTK("\n深圳雄帝读卡器暂时未实现PSAM指令函数");
	return -1;
}

UINT CJTReader::PSAM_Atr(BYTE bNode,BYTE &brLen,char *strATR)
{
	PRINTK("\n深圳雄帝读卡器暂时未实现PSAM复位函数");
	return -1;
}


UINT CJTReader::SecureRead(BYTE bKeyIndex,BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE &bRetFileLen,char *strResp)
{
	PRINTK("\n深圳雄帝读卡器暂时未实现SecureRead函数");
	return -1;
}

UINT CJTReader::Beep(BYTE bAct)
{
	PRINTK("\n深圳雄帝读卡器暂时未实现Beep函数");
	return -1;
}