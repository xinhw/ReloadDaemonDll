/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/19 10:32:26

    Reversion:
			航天金卡读卡器接口。
-------------------------------------------------------------------------*/
#pragma once
#include "CardReader.h"

class CAISINOReader : public CCardReader
{
public:
	CAISINOReader(void);
	~CAISINOReader(void);
private:

	HMODULE m_hDll;
	long m_hDevice;

	typedef int (WINAPI *LPFN_JT_OpenReader)(int iComID, const char *szParas);
	typedef int (WINAPI *LPFN_JT_CloseReader)(int iComID);
	typedef void (WINAPI *LPFN_JT_ReaderVersion)(int iComID,char* szReaderVersion,int iRVerMaxLength,char* szAPIVersion,int iAPIVerMaxLength);
	typedef int (WINAPI *LPFN_JT_GetLastError)(int iComID,char* szLastError,int iMaxLength);
	typedef int (WINAPI *LPFN_JT_OpenCard)(int iComID, unsigned char *pCardSerialNum);
	typedef int (WINAPI *LPFN_JT_CloseCard)(int iComID);
	typedef int (WINAPI *LPFN_JT_AudioControl)(int iComID,unsigned char cTimes,unsigned char cVoice);
	typedef int (WINAPI *LPFN_JT_CPUCCommand)(int iComID,const unsigned char *pCommand,int iLenCmd,unsigned char *pReply,int *piLenRep);

	LPFN_JT_OpenReader lp_jt_openreader;
	LPFN_JT_CloseReader lp_jt_closereader;
	LPFN_JT_ReaderVersion lp_jt_readerversion;
	LPFN_JT_GetLastError lp_jt_getlasterror;
	LPFN_JT_OpenCard lp_jt_opencard;
	LPFN_JT_CloseCard lp_jt_closecard;
	LPFN_JT_AudioControl lp_jt_audiocontrol;
	LPFN_JT_CPUCCommand lp_jt_cpucommand;

	int initDll();

public:
	virtual UINT Open(char *strAddress, unsigned int iBaud);
	virtual void Close();
	virtual UINT Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult);
	virtual UINT RunCmd(char *strCmd, char *strResult);
	virtual UINT Halt();
};
