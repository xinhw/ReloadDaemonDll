/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created£º2018/07/24 16:13:10

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
	//	³õÊ¼»¯
	virtual int init(BYTE *elf15);
	//	¿¨Æ¬Çå³ý
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
	int updateELF0015(BYTE bVer,BYTE *szAPPID,BYTE *szFile0015);
	int updateELF0016(BYTE bVer,BYTE *szAPPID,BYTE *szFile0016);
	int updateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szNewValidDate);
	int updateELF000E(BYTE bVer,BYTE *szAPPID,BYTE *szFile000E);
	int reloadPIN(BYTE bVer,BYTE *szAPPID,BYTE bPINLen,BYTE *szPIN);

	int readAdfFile(BYTE bFID,BYTE bOffset, BYTE bLength, BYTE *szFile);
};