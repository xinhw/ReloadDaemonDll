/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/27 14:07:06

    Reversion:
			OBU����
-------------------------------------------------------------------------*/
#pragma once


class COBUCard: public CCPUCardBase
{
public:
	COBUCard();
	COBUCard(CCardReader *preader,ClsCommand *pcmd);
	~COBUCard();

public:
	//	��ʼ��
	int init(BYTE *elf01_mf,BYTE *elf01_adf);
	//	��Ƭ���
	int clear();

	int preInit(BYTE *elf01_mf);

private:
	int obu_load_key(BYTE bVer,BYTE *szAPPID,BYTE *szDID,BYTE bKeyNo,BYTE bKeyHeaderLen,BYTE *szKeyHeader);

public:
	int read_obu(BYTE *elf01_mk);
	int update_mf_elf01(BYTE bVer,BYTE *szAPPID,BYTE *szELF01);
	int update_adf_elf01(BYTE bVer,BYTE *szAPPID,BYTE *szELF01);
	int update_load_flag(BYTE bVer,BYTE *szAPPID,BYTE bFlag);
	int read_vechile_file(BYTE bNode,BYTE bVer,BYTE *szPlainFile);
	int getOBUUID(BYTE *szUID);
};