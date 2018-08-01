/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/24 16:12:29

    Reversion:
			天喻CPU卡类
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
	//	初始化
	int init(BYTE *elf15);
	//	卡片清除
	int clear(BYTE *elf15);
};