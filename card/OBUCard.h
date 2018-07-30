/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created£º2018/07/27 14:07:06

    Reversion:
        
-------------------------------------------------------------------------*/
#pragma once


class COBUCard: public CCPUCardBase
{
public:
	COBUCard();
	COBUCard(CCardReader *preader,ClsCommand *pcmd);
	~COBUCard();

public:
	//	³õÊ¼»¯
	int init(BYTE *elf01_mf,BYTE *elf01_adf);
	//	¿¨Æ¬Çå³ý
	int clear();
private:
	int obu_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);

public:
	int read_obu(BYTE *elf01_mk,BYTE *elf01_adf01);
	int update_mf_elf01(BYTE bVer,BYTE *szAPPID,BYTE *szELF01);
	int update_adf_elf01(BYTE bVer,BYTE *szAPPID,BYTE *szELF01);
	int update_load_flag(BYTE bVer,BYTE *szAPPID,BYTE bFlag);
};