#pragma once



class CJD3DESCard: public CCPUCardBase
{
public:
	CJD3DESCard();
	CJD3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CJD3DESCard();

private:
	int jd_load_key(BYTE bIndex,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);
public:
	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
};
