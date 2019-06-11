
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ETCCNSPCOMM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ETCCNSPCOMM_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef ETCCNSPCOMM_EXPORTS
#define ETCCNSPCOMM_API __declspec(dllexport)
#else
#define ETCCNSPCOMM_API __declspec(dllimport)
#endif

#include "etcCNSPCard.hpp"


/*	��������ʵ��*/
CTcpTransfer *ptransfer;
/*	��̨��Կ����ʵ��*/
ClsCommand *pcmd;

bool validation();


extern "C"
{
	/***************************************************************************************/
	/*				ͨ�ú���															   */
	/***************************************************************************************/
	/*1. ���ӵ�������Կ����ǰ�ã� Online Keys Service
		strip	[in]	ǰ�õ�IP��ַ
		wport	[in]	ǰ�õ�PORT
	*/
	int __stdcall connectOKS(char *strip,WORD wport);

	/*2. �رյ�������Կ����ǰ�õ�����*/
	int __stdcall disconnectOKS();

	void __stdcall setBankID(char *strbankid);
	void __stdcall setAgentCode(char *stragentcode);

	//4.1	����Աǩ����֤1031
	int	__stdcall 	cmd_1031(BYTE *szOperatorNo);

	//4.2	�û���Ȧ����֤1032
	int	__stdcall 	cmd_1032(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
					BYTE *szMAC1,
					BYTE *szMAC);

	//4.3	�û������η�����֤1033
	int	__stdcall 	cmd_1033(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.4	OBU���η�����֤1034
	int	__stdcall 	cmd_1034(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.5	�û����޸���Ч����֤1035
	int	__stdcall 	cmd_1035(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.6	�û���������֤ 1036
	int	__stdcall 	cmd_1036(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
					BYTE *szMAC);

	//4.7	�û�������TAC��֤ 1037
	int	__stdcall 	cmd_1037(BYTE bVer,BYTE *szAPPID,
					DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD nAuditNo,BYTE *szDateTime,BYTE *szTAC);

	//4.8	OBU�޸Ĳ�ж��־��֤1038
	int	__stdcall 	cmd_1038(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.9	�û���PIN��װ��֤1039
	int	__stdcall 	cmd_1039(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.10	�û�����ȡ��Կ1040
	int	__stdcall 	cmd_1040(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);

	//4.11	�û������η��в����ļ���֤1041
	int	__stdcall 	cmd_1041(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.12	OBU��ȡ��Կ1042
	int	__stdcall 	cmd_1042(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);


	//4.13	����ͨ�п���KeyA��Կ 2011
	int	__stdcall 	cmd_2011(BYTE *szCityCode,BYTE *szSerialNo,BYTE *szAuditNo,BYTE *szCardMAC,BYTE bKeyNum,BYTE *szSectorID,
					BYTE *szKey);
	
	//4.14	OBU������Ϣ�ļ����� 1043
	int	__stdcall	cmd_1043(BYTE bVer,BYTE *szAPPID,BYTE bKeyIndex,BYTE bInLen,BYTE *szEncData,
					BYTE *bOutLen,BYTE *szData);

	//4.15	PSAM����Ȩ 1044
	int	__stdcall 	cmd_1044(BYTE *szSAMNo,BYTE *szRnd,
					DWORD dwRoadID,char *strRoadName,
					DWORD dwStationID, char *strStationName,BYTE bStationType,
					BYTE bLaneType,BYTE bLaneID,
					BYTE *bAPDULen,BYTE *szAPDU,char *strListNo);

	//4.16	PSAM��������Ȩȷ�� 1045
	int	__stdcall 	cmd_1045(BYTE *szSAMNo,char *strListNo,
					WORD wSW1SW2,BYTE bResult);

	//4.17	PSAM������ǩ�� 1046
	int	__stdcall 	cmd_1046(BYTE *szSAMNo,BYTE *szTerminalNo,
					DWORD dwRoadID,char *strRoadName,
					DWORD dwStationID, char *strStationName,BYTE bStationType,
					BYTE bLaneType,BYTE bLaneID,
					BYTE *szTerminalTime,
					char *strListNo);

};
