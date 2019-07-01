#pragma once


class CWD3DESCard: public CCPUCardBase
{
public:
	CWD3DESCard();
	CWD3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CWD3DESCard();

private:
	BYTE m_szPKey[16];

	int wd_secure_update_key(BYTE bKeyType,BYTE bKeyVer,BYTE *szKey);

	int wd_load_key(BYTE bIndex,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *szKey=NULL);
public:
	//	³õÊ¼»¯
	int init(BYTE *elf15);
	//	¿¨Æ¬Çå³ý
	int clear(BYTE *elf15);
};