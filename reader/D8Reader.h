/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created£º2019/08/27 13:36:52

    Reversion:
        	µÂ¿¨D8(D3)¶Á¿¨Æ÷
-------------------------------------------------------------------------*/
#pragma once
#include "CardReader.h"

class CD8Reader : public CCardReader
{
public:
	CD8Reader(void);
	~CD8Reader(void);
private:
	HANDLE m_hDevice;
	int getCard(UCHAR *pszSerialNO,BYTE &bSNOLen,BYTE &bATS);
public:
	virtual UINT Open(char *strAddress, unsigned int iBaud);
	virtual void Close();
	virtual UINT Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult);
	virtual UINT RunCmd(char *strCmd, char *strResult);
	virtual UINT Halt();
	virtual UINT PSAM_RunCmd(char *strCmd, char *strResult);
	virtual UINT PSAM_Atr(BYTE bNode,BYTE &brLen,char *strATR);
	virtual UINT SecureRead(BYTE bKeyIndex,BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE &bRetFileLen,char *strResp);
	virtual UINT Beep(BYTE bAct);
};
