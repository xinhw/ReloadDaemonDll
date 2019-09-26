#pragma once


class COBUCardSAM: public COBUCard
{
public:
	COBUCardSAM();
	COBUCardSAM(CCardReader *preader,ClsCommand *pcmd);
	~COBUCardSAM();

public:
	//	³õÊ¼»¯
	int init(BYTE *elf01_mf,BYTE *elf01_adf);
	//	¿¨Æ¬Çå³ý
	int clear();
	
	int preInit(WORD wDFID,BYTE *elf01_mf);

private:
	int obu_load_key(char *sAPDU,char *strKeyHeader,char *strPID,char *strDID);
	BYTE m_bSAMNode;

public:
	int read_obu(BYTE *elf01_mk);
	int update_mf_elf01(char *strKeyHeader,char *strDID,char *strAPDU);
	int update_adf_elf01(char *strKeyHeader,char *strDID,char *strAPDU);
	int update_load_flag(BYTE bVer,BYTE *szAPPID,BYTE bFlag);
	int read_vechile_file(BYTE bNode,BYTE bVer,BYTE *szPlainFile);
	int getOBUUID(BYTE *szUID);
	int unlockapp(BYTE bVer,BYTE *szAPPID);

	void setSAMNode(BYTE bNode);

};