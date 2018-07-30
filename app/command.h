/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/16 17:01:55

    Reversion:
        
-------------------------------------------------------------------------*/
#ifndef		__CCOMMMAND_H__
#define		__CCOMMMAND_H__


class ClsCommand
{
public:
	ClsCommand();
	ClsCommand(CTcpTransfer *pt);
	~ClsCommand();

public:
	//4.1	操作员签到认证1031
	int		cmd_1031(BYTE *szOperatorNo);

	//4.2	用户卡圈存认证1032
	int		cmd_1032(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
					BYTE *szMAC1,
					BYTE *szMAC);

	//4.3	用户卡二次发行认证1033
	int		cmd_1033(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.4	OBU二次发行认证1034
	int		cmd_1034(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.5	用户卡修改有效期认证1035
	int		cmd_1035(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.6	用户卡消费认证 1036
	int		cmd_1036(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
					BYTE *szMAC);

	//4.7	用户卡消费TAC验证 1037
	int		cmd_1037(BYTE bVer,BYTE *szAPPID,
					DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD nAuditNo,BYTE *szDateTime,BYTE *szTAC);

	//4.8	OBU修改拆卸标志认证1038
	int		cmd_1038(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.9	用户卡PIN重装认证1039
	int		cmd_1039(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.10	用户卡获取密钥1040
	int		cmd_1040(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);

	//4.11	用户卡二次发行补充文件认证1041
	int		cmd_1041(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.12	OBU获取密钥1042
	int		cmd_1042(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);


	//4.13	计算通行卡的KeyA密钥 2011
	int		cmd_2011(BYTE *szCityCode,BYTE *szSerialNo,BYTE *szAuditNo,BYTE *szCardMAC,BYTE bKeyNum,BYTE *szSectorID,
					BYTE *szKey);
	

private:
	PACKAGEHEADER	m_ph;
	void	updateHeader(WORD wType,DWORD wDataLen);
	int		send_recv(WORD wTransType,
					  WORD	wLenIn,UCHAR	*pszBufIn,
					  WORD	*wLenOut,UCHAR	*pszBufOut,UINT	nWaitMaxTime = 3);
	int		getKey(WORD wType,BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);

private:
	char	m_strBankID[16];		//	银行网点唯一编号
	WORD	m_wRemainCount;			//	针对本设备的剩余次数；
	char	m_strAgentCode[7];		//	代理商代码，由高速公路管理单位统一分配 
	CTcpTransfer *m_ptransfer;
	void	init();

public:
	char	*getAgentCode();
	void	setAgentCode(char *s);

	char	*getBankID();
	void	setBankID(char *s);

	WORD	getRemainCount();

	PPACKAGEHEADER getHeader();

public:
	void	setTCPTransfer(CTcpTransfer *pt);

};









#endif

