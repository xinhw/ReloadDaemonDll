/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/16 17:01:55

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
	//4.1	����Աǩ����֤1031
	int		cmd_1031(BYTE *szOperatorNo);

	//4.2	�û���Ȧ����֤1032
	int		cmd_1032(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
					BYTE *szMAC1,
					BYTE *szMAC);

	//4.3	�û������η�����֤1033
	int		cmd_1033(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.4	OBU���η�����֤1034
	int		cmd_1034(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.5	�û����޸���Ч����֤1035
	int		cmd_1035(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.6	�û���������֤ 1036
	int		cmd_1036(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
					BYTE *szMAC);

	//4.7	�û�������TAC��֤ 1037
	int		cmd_1037(BYTE bVer,BYTE *szAPPID,
					DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD nAuditNo,BYTE *szDateTime,BYTE *szTAC);

	//4.8	OBU�޸Ĳ�ж��־��֤1038
	int		cmd_1038(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.9	�û���PIN��װ��֤1039
	int		cmd_1039(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.10	�û�����ȡ��Կ1040
	int		cmd_1040(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);

	//4.11	�û������η��в����ļ���֤1041
	int		cmd_1041(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.12	OBU��ȡ��Կ1042
	int		cmd_1042(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);


	//4.13	����ͨ�п���KeyA��Կ 2011
	int		cmd_2011(BYTE *szCityCode,BYTE *szSerialNo,BYTE *szAuditNo,BYTE *szCardMAC,BYTE bKeyNum,BYTE *szSectorID,
					BYTE *szKey);
	
	//4.14	OBU������Ϣ�ļ����� 1043
	int		cmd_1043(BYTE bVer,BYTE *szAPPID,BYTE bKeyIndex,BYTE bInLen,BYTE *szEncData,
					BYTE *bOutLen,BYTE *szData);

	//4.15	PSAM����Ȩ 1044
	int		cmd_1044(BYTE *szSAMNo,BYTE *szRnd,
					DWORD dwRoadID,char *strRoadName,
					DWORD dwStationID, char *strStationName,BYTE bStationType,
					BYTE bLaneType,BYTE bLaneID,
					BYTE *bAPDULen,BYTE *szAPDU,char *strListNo);

	//4.16	PSAM��������Ȩȷ�� 1045
	int		cmd_1045(BYTE *szSAMNo,char *strListNo,
					WORD wSW1SW2,BYTE bResult);

	//4.17	PSAM������ǩ�� 1046
	int		cmd_1046(BYTE *szSAMNo,BYTE *szTerminalNo,
					DWORD dwRoadID,char *strRoadName,
					DWORD dwStationID, char *strStationName,BYTE bStationType,
					BYTE bLaneType,BYTE bLaneID,
					BYTE *szTerminalTime,
					char *strListNo);

	// 4.16.1	CPC����������1047
	int		cmd_1047(BYTE bKeyVer,
					BYTE *szSNO,
					BYTE *szAPPID,
					BYTE *szIssuer,
					BYTE *szStartDate,BYTE *szEndDate,
					BYTE bCardVer,
					char *strListNo);


	// 4.16.2	CPC����ȡ��Կ1048
	int		cmd_1048(char *strListNo,
					BYTE bKeyNo,
					BYTE *szRand,
					WORD wSW1SW2,
					BYTE bResult,
					BYTE *bLen,BYTE *szEncKey);

	// 4.16.3	CPC�������ļ�1049
	int		cmd_1049(char *strListNo,
					BYTE *szRand,
					WORD wSW1SW2,
					BYTE bResult,
					BYTE *szMAC);

	// 4.16.4	CPC�����½������1050
	int		cmd_1050(char *strListNo,
					WORD wSW1SW2,
					BYTE bResul);


private:
	PACKAGEHEADER	m_ph;
	void	updateHeader(WORD wType,DWORD wDataLen);
	int		send_recv(WORD wTransType,
					  WORD	wLenIn,UCHAR	*pszBufIn,
					  WORD	*wLenOut,UCHAR	*pszBufOut,UINT	nWaitMaxTime);

	int		__send_recv(WORD wTransType,
					  WORD	wLenIn,UCHAR	*pszBufIn,
					  WORD	*wLenOut,UCHAR	*pszBufOut,UINT	nWaitMaxTime);

	int		getKey(WORD wType,BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);

private:
	char	m_strBankID[20];		//	��������Ψһ���
	WORD	m_wRemainCount;			//	��Ա��豸��ʣ�������
	char	m_strAgentCode[7];		//	�����̴��룬�ɸ��ٹ�·����λͳһ���� 

	CTcpTransfer *m_ptransfer;
	void	init();

	DWORD	m_dwTimeout;

public:
	char	*getAgentCode();
	void	setAgentCode(char *s);

	char	*getBankID();
	void	setBankID(char *s);

	WORD	getRemainCount();

	PPACKAGEHEADER getHeader();

public:
	//	2018-08-27: ����ͨ�ų�ʱ
	void	setWaitTimeout(DWORD dwtm);
	DWORD	getWaitTimeout();
};

#endif

