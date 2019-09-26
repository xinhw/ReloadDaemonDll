/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2019/08/27 13:39:56

    Reversion:
        	CPC��
-------------------------------------------------------------------------*/
#pragma once

class CCPCCard: public CCPUCardBase
{
public:
	CCPCCard();
	CCPCCard(CCardReader *preader,ClsCommand *pcmd);
	~CCPCCard();

private:
	int		cpc_load_key(BYTE bKeyNo,BYTE *szAPPID,BYTE *bP1P2);

public:
	char	m_strListNo[37];

	//	��ʼ��
	int init(BYTE *elf15);
	//	��Ƭ���
	int clear(BYTE *elf15);
	//	Ԥ����
	int preInit(WORD wDFID,BYTE *elf15);
};