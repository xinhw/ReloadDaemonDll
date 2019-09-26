/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/08/06 12:37:41

    Reversion:
			����3DES��
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
	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
	//	Ԥ����
	int preInit(WORD wDFID,BYTE *elf15);
};
