/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created£º2019/08/27 13:35:35

    Reversion:
        
-------------------------------------------------------------------------*/

#include "dcrf32.h"


/*-------------------------------------------------------------------------
Function:		CD8Reader.CD8Reader
Created:		2019-08-27 13:35:43
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CD8Reader::CD8Reader()
{
	m_hDevice = INVALID_HANDLE_VALUE;
}



/*-------------------------------------------------------------------------
Function:		CD8Reader.~CD8Reader
Created:		2019-08-27 13:35:48
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
CD8Reader::~CD8Reader()
{
	Close();
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.Open
Created:		2019-08-27 13:35:54
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::Open(char *strAddress, unsigned int iBaud)
{
	m_hDevice = dc_init(100,iBaud);
	if(m_hDevice<=0)
	{
		PRINTK("\n´ò¿ªÉÏµÄµÂ¿¨D8¶Á¿¨Æ÷Ê§°Ü:%d",m_hDevice);
		return 0x1A14;
	}
	//dc_beep(m_hDevice,40);

	return 0;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.Close
Created:		2019-08-27 13:36:02
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void CD8Reader::Close()
{

	if(m_hDevice==INVALID_HANDLE_VALUE) return;

	dc_exit(m_hDevice);
	m_hDevice = INVALID_HANDLE_VALUE;

	return;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.getCard
Created:		2019-08-27 13:36:05
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int CD8Reader::getCard(UCHAR *pszSerialNO,BYTE &bSNOLen,BYTE &bATS)
{
	int		i,ret;
	UCHAR	pszBuf[64];
	BYTE	bSAK;
	UINT	wSNO;
	WORD	wATQA;

	bATS= 0;
	memset(pszBuf,0x00,64);
	bSNOLen = 4;

	for(i=0;i<5;i++)
	{
		dc_reset (m_hDevice,20);

		wATQA = 0x0000;
		ret = dc_request(m_hDevice,0x00,&wATQA);
		if(ret) 
		{
			PRINTK("\nRequestÊ§°Ü");
			continue;
		}

		ret = dc_anticoll(m_hDevice,0x00,&wSNO);
		if(ret==0)
		{
			CMisc::Int2Bytes(wSNO,pszBuf);
		}
		else
		{
			PRINTK("\nAncticoll Level1 Ê§°Ü");
			continue;
		}

		ret = dc_select(m_hDevice,wSNO,&bSAK);
		if(ret) 
		{
			PRINTK("\nSelect Level1 Ê§°Ü");
			continue;
		}

		ret = bSAK;

		//	PICC Compliant with ISO/IEC 14443-4
		ret = bSAK>>5;
		ret = ret&0x01;
		
		bATS = 0;
		if(ret) 
		{
			bATS = 1;		//	PICC Compliant with ISO/IEC 14443-4
		}

		//	UID Compelte
		ret = bSAK>>2;
		ret = ret & 0x01;

		if(ret==0x00)		//	UID Complete
		{
			memcpy(pszSerialNO,pszBuf,4);
			return 0;
		}
		else				//	UID not complete
		{
			memcpy(pszSerialNO,pszBuf+1,3);
		}

		ret = dc_anticoll2(m_hDevice,0x00,&wSNO);
		if(ret==0)
		{
			CMisc::Int2Bytes(wSNO,pszBuf);
		}
		else
		{
			PRINTK("\nAncticoll Level2 Ê§°Ü");
			continue;
		}

		ret = dc_select2(m_hDevice,wSNO,&bSAK);
		if(ret) 
		{
			PRINTK("\nSelect Level2 Ê§°Ü");
			continue;
		}
		else
		{
			//	PICC Compliant with ISO/IEC 14443-4
			ret = bSAK>>5;
			ret = ret&0x01;
			bATS = 0;
			if(ret) bATS = 1;

			memcpy(pszSerialNO+3,pszBuf,4);
			bSNOLen = 7;

			return 0;
		}		
	}
	
	return -1;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.Initialize
Created:		2019-08-27 13:36:13
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult)
{
	int ret;
	BYTE szSNO[32];
	BYTE bATS = 0,bSNOLen;

	memset(szSNO,0x00,32);

	//dc_reset(m_hDevice,100);

	ret = getCard(szSNO,bSNOLen,bATS);
	if(ret)
	{
		PRINTK("\nÑ¯¿¨Ê§°Ü:%d",ret);
		return ret;
	}
	memcpy(strsno,szSNO,bSNOLen);
	
	if(!bATS) return 0;

	memset(szSNO,0x00,32);
	ret = dc_pro_reset(m_hDevice,&bATSLen,strResult);
	if(ret)
	{
		PRINTK("\nATSÊ§°Ü:%d",ret);
		return ret;
	}

	return 0;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.RunCmd
Created:		2019-08-27 13:36:21
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::RunCmd(char *strCmd, char *strResult)
{
	int ret;
	BYTE pszCmd[256],pszBuf[256],buf[5];
	BYTE ilen,bCmdLen;
	BYTE sw1,sw2;

	memset(pszCmd,0x00,256);
#ifdef DEBUG_PRINT
	PRINTK("\nCMD:%s",strCmd);
#endif

	bCmdLen = strlen(strCmd)/2;
	CMisc::StringToByte(strCmd, pszCmd);

	ilen = 0;
	memset(pszBuf,0x00,256);
	ret = dc_pro_command(m_hDevice,bCmdLen,pszCmd,&ilen,pszBuf,7);
	if(ret) return ret;

	sw1 = pszBuf[ilen-2];
	sw2 = pszBuf[ilen-1];
	if(sw1==0x61)
	{
		memset(buf,0x00,5);
		buf[1] = 0xc0;buf[4]=sw2;

		ilen = 0;
		memset(pszBuf,0x00,256);

		ret = dc_pro_command(m_hDevice,5,buf,&ilen,pszBuf,7);
		if(ret) return ret;
	}

	sw1 = pszBuf[ilen-2];
	sw2 = pszBuf[ilen-1];
	
	ret = sw1;
	ret = ret*0x100 + sw2;

	CMisc::ByteToString(pszBuf,ilen,strResult);
#ifdef DEBUG_PRINT
	PRINTK("\nRSP:%s",strResult);
#endif

	if(ret==0x9000) return 0;

	return ret;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.Halt
Created:		2019-08-27 13:36:28
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::Halt()
{
	dc_pro_halt(m_hDevice);
	return 0;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.PSAM_RunCmd
Created:		2019-08-27 13:36:33
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::PSAM_RunCmd(char *strCmd, char *strResult)
{
	PRINTK("\nµÂ¿¨D8¶Á¿¨Æ÷ÔÝÊ±Î´ÊµÏÖPSAMÖ¸Áîº¯Êý");
	return -1;
}

/*-------------------------------------------------------------------------
Function:		CD8Reader.PSAM_Atr
Created:		2019-08-27 13:36:38
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::PSAM_Atr(BYTE bNode,BYTE &brLen,char *strATR)
{
	PRINTK("\nµÂ¿¨D8¶Á¿¨Æ÷ÔÝÊ±Î´ÊµÏÖPSAM¸´Î»º¯Êý");
	return -1;
}

/*-------------------------------------------------------------------------
Function:		CD8Reader.SecureRead
Created:		2019-08-27 13:36:42
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::SecureRead(BYTE bKeyIndex,BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE &bRetFileLen,char *strResp)
{
	PRINTK("\nµÂ¿¨D8¶Á¿¨Æ÷ÔÝÊ±Î´ÊµÏÖSecureReadº¯Êý");
	return -1;
}


/*-------------------------------------------------------------------------
Function:		CD8Reader.Beep
Created:		2019-08-27 13:36:45
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
UINT CD8Reader::Beep(BYTE bAct)
{

	if(bAct==0)
	{
		dc_beep(m_hDevice,40);
	}
	else
	{
		dc_beep(m_hDevice,10);
		Sleep(100);
		dc_beep(m_hDevice,10);
		Sleep(100);
		dc_beep(m_hDevice,10);
	}

	return 0;
}