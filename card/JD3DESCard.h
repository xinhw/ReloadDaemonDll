/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/07/31 15:00:50

    Reversion:
			�ݵ�3DES����ʼ�����ຣ�����ģ�
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
	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
	//	Ԥ����
	int preInit(WORD wDFID,BYTE *elf15);
};
