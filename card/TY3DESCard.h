/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/08/06 12:37:41

    Reversion:
			天喻3DES卡
-------------------------------------------------------------------------*/
#pragma once




class CTY3DESCard: public CCPUCardBase
{
public:
	CTY3DESCard();
	CTY3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CTY3DESCard();

private:
	int ty_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);

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
