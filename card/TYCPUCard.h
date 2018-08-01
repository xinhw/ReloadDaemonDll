/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/24 16:12:29

    Reversion:
			����CPU����
-------------------------------------------------------------------------*/
#pragma once


class CTYCPUCard: public CCPUCardBase
{
public:
	CTYCPUCard();
	CTYCPUCard(CCardReader *preader,ClsCommand *pcmd);
	~CTYCPUCard();

private:
	int ty_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);
public:
	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
};