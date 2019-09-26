/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2019/08/27 13:39:56

    Reversion:
        	CPC卡
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

	//	初始化
	int init(BYTE *elf15);
	//	卡片清除
	int clear(BYTE *elf15);
	//	预处理
	int preInit(WORD wDFID,BYTE *elf15);
};