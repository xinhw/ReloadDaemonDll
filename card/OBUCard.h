#pragma once


class COBUCard: public CCPUCardBase
{
public:
	COBUCard();
	COBUCard(CCardReader *preader,ClsCommand *pcmd);
	~COBUCard();

public:
	//	初始化
	int init(BYTE *elf15);
	//	个人化
	int personalize(BYTE *elf15,BYTE *elf16);
	//	卡片清除
	int clear(BYTE *elf15);

public:
	int read_obu(BYTE *elf01_mk,BYTE *elf01_adf01);
	
};