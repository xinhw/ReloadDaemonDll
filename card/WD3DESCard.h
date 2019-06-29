#pragma once


class CWD3DESCard: public CCPUCardBase
{
public:
	CWD3DESCard();
	CWD3DESCard(CCardReader *preader,ClsCommand *pcmd);
	~CWD3DESCard();

private:
	int wd_load_key(BYTE bIndex,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader,BYTE *szKey=NULL);
public:
	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
};