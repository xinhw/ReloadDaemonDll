/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/07/31 15:00:50

    Reversion:
			捷德3DES卡初始化（青海、宁夏）
-------------------------------------------------------------------------*/
#pragma once



class CJD3DESCard: public CCPUCardBase
{
public:
	CJD3DESCard();
	CJD3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CJD3DESCard();

private:
	int jd_load_key_secure(BYTE bIndex,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);
	int jd_load_key(BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *szKey);

	WORD m_wNetworkID;
	BYTE m_szPKey[16];

public:
	//	初始化
	int init(BYTE *elf15);
	//	卡片清除
	int clear(BYTE *elf15);
	//	预处理
	int preInit(WORD wDFID,BYTE *elf15);
};
