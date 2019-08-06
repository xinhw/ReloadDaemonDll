/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/08/06 12:38:19

    Reversion:
        ������3DES��
-------------------------------------------------------------------------*/
#pragma once


class CCTD3DESCard: public CCPUCardBase
{
public:
	CCTD3DESCard();
	CCTD3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CCTD3DESCard();

private:
	BYTE m_szPKey[16];

	int ctd_load_key(BYTE bIndex,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *P1P2);

	WORD m_wNetworkID;
public:
	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
	//	Ԥ����
	int preInit(WORD wDFID,BYTE *elf15);
};