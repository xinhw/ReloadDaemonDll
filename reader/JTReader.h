/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created£º2018/07/31 15:59:53

    Reversion:
        
-------------------------------------------------------------------------*/
#pragma once
#include "CardReader.h"

class CJTReader : public CCardReader
{
public:
	CJTReader();
	~CJTReader();

private:
	int m_icom;

	HMODULE m_hDll;

	typedef int (WINAPI *LPFN_JT_OpenReader)(int iComID, const char *szParas);
	typedef int (WINAPI *LPFN_JT_CloseReader)(int iComID);
	typedef int (WINAPI *LPFN_JT_OpenCard)(int iComID, unsigned char *pCardSerialNum,int *piCardSerialNumLength);
	typedef int (WINAPI *LPFN_JT_CloseCard)(int iComID);
	typedef int (WINAPI *LPFN_JT_CPUCCommand)(int iComID,const unsigned char *pCommand,int iLenCmd,unsigned char *pReply,int *piLenRep);

	LPFN_JT_OpenReader lp_jt_openreader;
	LPFN_JT_CloseReader lp_jt_closereader;
	LPFN_JT_OpenCard lp_jt_opencard;
	LPFN_JT_CloseCard lp_jt_closecard;
	LPFN_JT_CPUCCommand lp_jt_cpucommand;

	int initDll();
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