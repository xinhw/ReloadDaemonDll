
#ifndef		__POSAPP_H_
#define		__POSAPP_H_


class ClsReload
{
public:
	ClsReload();
	~ClsReload();
	
public:
	int			process_request(  PPACKAGEHEADER ph,const WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	

private:

	//3.1	操作员签到认证1031（一期不实现）
	int		cmd_1031(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.2	充资认证1032
	int		cmd_1032(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.3	CPU卡二次发行认证1033
	int		cmd_1033(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.4	OBU二次发行认证1034
	int		cmd_1034(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.5	修改储值卡有效期认证1035
	int		cmd_1035(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.6	消费认证 1036
	int		cmd_1036(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.7	消费TAC验证 1037
	int		cmd_1037(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.8	拆卸标志修改1038
	int		cmd_1038(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.9	PIN解锁认证1039
	int		cmd_1039(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.10	CPU卡密钥获取1040
	int		cmd_1040(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.11	CPU卡二次发行补充文件认证1041
	int		cmd_1041(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.12	计算通行卡的KeyA密钥 2011
	int		cmd_2011(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	CString   retMessage( int  i);

private:
	WORD			wErrorCode;
	DWORD			nTransNo;
	PACKAGEHEADER	m_ph;


private:
	char	m_strBankID[20];		//	银行网点唯一编号
	BYTE	m_pszAPPID[8];			//	用户卡卡号orOBU卡号；
	WORD	m_wRemainCount;			//	针对本设备的剩余次数；


};
#endif

