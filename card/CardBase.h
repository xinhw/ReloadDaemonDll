/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/24 16:13:10

    Reversion:
        
-------------------------------------------------------------------------*/
#pragma once


class CCPUCardBase
{
public:
	CCPUCardBase();
	CCPUCardBase(CCardReader *preader,ClsCommand *pcmd);

	~CCPUCardBase();

protected:
	ClsCommand *m_pCmd;
	CCardReader *m_pReader;
	bool	validation();

public:
	//	初始化
	virtual int init(BYTE *elf15);
	//	个人化
	virtual int personalize(BYTE *elf15,BYTE *elf16);
	//	卡片清除
	virtual int clear(BYTE *elf15);

public:
	int rats(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS);
	int readCard(BYTE *elf15,BYTE *elf16,DWORD &dwRemain);
	int readRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec);

	int credit(BYTE bVer,BYTE *szAPPID,BYTE *szDeviceNo,DWORD dwAmount,
				WORD &wSeqNo,
				BYTE *szTransTime,
				BYTE *szTAC);
	
	int debit(BYTE bVer,BYTE *szAPPID,BYTE *szDeviceNo,DWORD dwAmount,DWORD dwAuditNo,BYTE *szTransTime,
				WORD &wSeqNo,				
				BYTE *szTAC);

};