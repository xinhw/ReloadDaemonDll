/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/07/31 15:01:41

    Reversion:
			握奇3DES卡初始化
-------------------------------------------------------------------------*/
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

	WORD m_wNetworkID;
public:
	//	初始化
	int init(BYTE *elf15);
	//	卡片清除
	int clear(BYTE *elf15);
	//	预处理
	int preInit(WORD wDFID,BYTE *elf15);
};