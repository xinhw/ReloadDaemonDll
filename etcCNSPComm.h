
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


/*	网络连接实例*/
CTcpTransfer *ptransfer;
/*	后台密钥服务实例*/
ClsCommand *pcmd;

bool validation();


extern "C"
{
	/***************************************************************************************/
	/*				通用函数															   */
	/***************************************************************************************/
	/*1. 连接到在线密钥服务前置： Online Keys Service
		strip	[in]	前置的IP地址
		wport	[in]	前置的PORT
	*/
	int __stdcall connectOKS(char *strip,WORD wport);

	/*2. 关闭到在线密钥服务前置的连接*/
	int __stdcall disconnectOKS();

	void __stdcall setBankID(char *strbankid);
	void __stdcall setAgentCode(char *stragentcode);

	//4.1	操作员签到认证1031
	int	__stdcall 	cmd_1031(BYTE *szOperatorNo);

	//4.2	用户卡圈存认证1032
	int	__stdcall 	cmd_1032(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
					BYTE *szMAC1,
					BYTE *szMAC);

	//4.3	用户卡二次发行认证1033
	int	__stdcall 	cmd_1033(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.4	OBU二次发行认证1034
	int	__stdcall 	cmd_1034(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bFileType,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.5	用户卡修改有效期认证1035
	int	__stdcall 	cmd_1035(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.6	用户卡消费认证 1036
	int	__stdcall 	cmd_1036(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
					BYTE *szMAC);

	//4.7	用户卡消费TAC验证 1037
	int	__stdcall 	cmd_1037(BYTE bVer,BYTE *szAPPID,
					DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD nAuditNo,BYTE *szDateTime,BYTE *szTAC);

	//4.8	OBU修改拆卸标志认证1038
	int	__stdcall 	cmd_1038(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.9	用户卡PIN重装认证1039
	int	__stdcall 	cmd_1039(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.10	用户卡获取密钥1040
	int	__stdcall 	cmd_1040(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);

	//4.11	用户卡二次发行补充文件认证1041
	int	__stdcall 	cmd_1041(BYTE bVer,BYTE *szAPPID,
					BYTE *szRnd,
					BYTE bCmdLen,
					BYTE *szCmd,
					BYTE *szMAC);

	//4.12	OBU获取密钥1042
	int	__stdcall 	cmd_1042(BYTE bVer,BYTE *szAPPID,
						BYTE bKeyNo,
						BYTE *szRnd,
						BYTE *szAPDU,
						BYTE bKeyHeaderLen,BYTE *szKeyHeader,
						BYTE *szPDID,
						BYTE *szEncKey,
						BYTE *szMAC);


	//4.13	计算通行卡的KeyA密钥 2011
	int	__stdcall 	cmd_2011(BYTE *szCityCode,BYTE *szSerialNo,BYTE *szAuditNo,BYTE *szCardMAC,BYTE bKeyNum,BYTE *szSectorID,
					BYTE *szKey);
	
	//4.14	OBU车辆信息文件解密 1043
	int	__stdcall	cmd_1043(BYTE bVer,BYTE *szAPPID,BYTE bKeyIndex,BYTE bInLen,BYTE *szEncData,
					BYTE *bOutLen,BYTE *szData);

	//4.15	PSAM卡授权 1044
	int	__stdcall 	cmd_1044(BYTE *szSAMNo,BYTE *szRnd,
					DWORD dwRoadID,char *strRoadName,
					DWORD dwStationID, char *strStationName,BYTE bStationType,
					BYTE bLaneType,BYTE bLaneID,
					BYTE *bAPDULen,BYTE *szAPDU,char *strListNo);

	//4.16	PSAM卡在线授权确认 1045
	int	__stdcall 	cmd_1045(BYTE *szSAMNo,char *strListNo,
					WORD wSW1SW2,BYTE bResult);

	//4.17	PSAM卡在线签到 1046
	int	__stdcall 	cmd_1046(BYTE *szSAMNo,BYTE *szTerminalNo,
					DWORD dwRoadID,char *strRoadName,
					DWORD dwStationID, char *strStationName,BYTE bStationType,
					BYTE bLaneType,BYTE bLaneID,
					BYTE *szTerminalTime,
					char *strListNo);

};
