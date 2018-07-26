#pragma once


class COBUCard: public CCPUCardBase
{
public:
	COBUCard();
	COBUCard(CCardReader *preader,ClsCommand *pcmd);
	~COBUCard();

public:
	//	��ʼ��
	int init(BYTE *elf15);
	//	���˻�
	int personalize(BYTE *elf15,BYTE *elf16);
	//	��Ƭ���
	int clear(BYTE *elf15);

public:
	int read_obu(BYTE *elf01_mk,BYTE *elf01_adf01);
	
};