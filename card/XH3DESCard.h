/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created£º2019/08/01 14:47:37

    Reversion:
        Öéº£ÐÇºº3DES¿¨Æ¬£¨ÄþÏÄ£©
-------------------------------------------------------------------------*/
#pragma once

class CXH3DESCard: public CCPUCardBase
{
public:
	CXH3DESCard();
	CXH3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CXH3DESCard();

private:
	BYTE m_szPKey[16];

	int xh_load_key(BYTE bIndex,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);

	WORD m_wNetworkID;
public:
	//	³õÊ¼»¯
	int init(BYTE *elf15);
	//	¿¨Æ¬Çå³ý
	int clear(BYTE *elf15);
	//	Ô¤´¦Àí
	int preInit(WORD wDFID,BYTE *elf15);
};