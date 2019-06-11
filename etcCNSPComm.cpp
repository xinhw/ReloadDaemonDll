// etcCNSPComm.cpp : Defines the entry point for the DLL application.
//

#include "etcCNSPComm.h"


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}






/***************************************************************************************/
/*				通用函数															   */
/***************************************************************************************/
/*1. 连接到在线密钥服务前置： Online Keys Service
	strip	[in]	前置的IP地址
	wport	[in]	前置的PORT
*/
/*-------------------------------------------------------------------------
Function:		connectOKS
Created:		2018-07-16 17:04:23
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall connectOKS(char *strip,WORD wport)
{
	int			ret;
	WSADATA		wsaData;  	
	char strBankID[20],strAgentCode[7];

	//	WinSock2初始化
	WORD wSockVersion = MAKEWORD(2,1);
	ret = WSAStartup(wSockVersion,&wsaData);
	if(ret) 
	{
		PRINTK("\nWIN SOCKET2初始化失败！");
		return ret;
	}

	if(NULL!=ptransfer)
	{
		ptransfer->disconnect();
		delete ptransfer;
		ptransfer=NULL;
	}

	ptransfer = new CTcpTransfer();
	ret = ptransfer->init_socket();
	if(ret)
	{
		PRINTK("\ninit_socket失败:%d",ret);
		return ret;
	}

	ret = ptransfer->connect_server(wport,strip);
	if(ret)
	{
		PRINTK("\nconnect_server连接服务器%s:%d失败:%d",strip,wport,ret);
		delete ptransfer;
		ptransfer = NULL;
		return ret;
	}

	if(NULL!=pcmd)
	{
		delete pcmd;
		pcmd = NULL;
	}
	pcmd = new ClsCommand(ptransfer);

	memset(strBankID,0x00,20);
	strcpy(strBankID,"5201301101512061652");
	pcmd->setBankID(strBankID);
	
	memset(strAgentCode,0x00,7);
	strcpy(strAgentCode,"668801");
	pcmd->setAgentCode(strAgentCode);

	return 0;
}

void __stdcall setBankID(char *strbankid)
{
	if(pcmd==NULL) return;
	if(strbankid==NULL) return;

	pcmd->setBankID(strbankid);	
	return;
}
void __stdcall setAgentCode(char *stragentcode)
{
	if(pcmd==NULL) return;
	if(stragentcode==NULL) return;

	pcmd->setAgentCode(stragentcode);
	return;
}

/*2. 关闭到在线密钥服务前置的连接*/
/*-------------------------------------------------------------------------
Function:		disconnectOKS
Created:		2018-07-16 17:04:27
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int __stdcall disconnectOKS()
{

	if(NULL!=ptransfer)
	{
		ptransfer->disconnect();
		delete ptransfer;
		ptransfer=NULL;
	}
	if(NULL!=pcmd)
	{
		delete pcmd;
		pcmd = NULL;
	}

	return 0;
}


/*-------------------------------------------------------------------------
Function:		validation
Created:		2018-07-28 10:58:19
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
bool validation()
{
	bool bOk = true;
	
	if(NULL==pcmd)
	{
		bOk = false;
	}


	return bOk;
}

//4.1	操作员签到认证1031
int	__stdcall 	cmd_1031(BYTE *szOperatorNo)
{
	if(!validation()) return -1;
	return pcmd->cmd_1031(szOperatorNo);

}

//4.2	用户卡圈存认证1032
int	__stdcall 	cmd_1032(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
				BYTE *szMAC1,
				BYTE *szMAC)
{

	if(!validation()) return -1;
	return pcmd->cmd_1032(bVer,szAPPID,
					szRnd,wSeqNo,nAmount,bTransFlag,szDeviceNo,szDateTime,dwRemain,
					szMAC1,
					szMAC);


}

//4.3	用户卡二次发行认证1033
int	__stdcall 	cmd_1033(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				BYTE bFileType,
				BYTE bCmdLen,
				BYTE *szCmd,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1033(bVer,szAPPID,
					szRnd,
					bFileType,
					bCmdLen,
					szCmd,
					szMAC);
}

//4.4	OBU二次发行认证1034
int	__stdcall 	cmd_1034(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				BYTE bFileType,
				BYTE bCmdLen,
				BYTE *szCmd,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1034(bVer,szAPPID,
					szRnd,
					bFileType,
					bCmdLen,
					szCmd,
					szMAC);
}

//4.5	用户卡修改有效期认证1035
int	__stdcall 	cmd_1035(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				BYTE bCmdLen,
				BYTE *szCmd,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1035(bVer,szAPPID,
					szRnd,
					bCmdLen,
					szCmd,
					szMAC);
}

//4.6	用户卡消费认证 1036
int	__stdcall 	cmd_1036(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1036(bVer,szAPPID,
				szRnd,
				wSeqNo,nAuditNo,nRemain,nAmount,bTransFlag,szDeviceNo,szDateTime,
				szMAC);
}

//4.7	用户卡消费TAC验证 1037
int	__stdcall 	cmd_1037(BYTE bVer,BYTE *szAPPID,
				DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD nAuditNo,BYTE *szDateTime,BYTE *szTAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1037(bVer,szAPPID,
				nAmount,bTransFlag,szDeviceNo,nAuditNo,szDateTime,szTAC);

}

//4.8	OBU修改拆卸标志认证1038
int	__stdcall 	cmd_1038(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				BYTE bCmdLen,
				BYTE *szCmd,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1038(bVer,szAPPID,
					szRnd,
					bCmdLen,
					szCmd,
					szMAC);
}

//4.9	用户卡PIN重装认证1039
int	__stdcall 	cmd_1039(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				BYTE bCmdLen,
				BYTE *szCmd,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1039(bVer,szAPPID,
					szRnd,
					bCmdLen,
					szCmd,
					szMAC);
}

//4.10	用户卡获取密钥1040
int	__stdcall 	cmd_1040(BYTE bVer,BYTE *szAPPID,
					BYTE bKeyNo,
					BYTE *szRnd,
					BYTE *szAPDU,
					BYTE bKeyHeaderLen,BYTE *szKeyHeader,
					BYTE *szPDID,
					BYTE *szEncKey,
					BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1040(bVer,szAPPID,
					bKeyNo,
					szRnd,
					szAPDU,
					bKeyHeaderLen,szKeyHeader,
					szPDID,
					szEncKey,
					szMAC);
}


//4.11	用户卡二次发行补充文件认证1041
int	__stdcall 	cmd_1041(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				BYTE bCmdLen,
				BYTE *szCmd,
				BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1041(bVer,szAPPID,
					szRnd,
					bCmdLen,
					szCmd,
					szMAC);
}

//4.12	OBU获取密钥1042
int	__stdcall 	cmd_1042(BYTE bVer,BYTE *szAPPID,
					BYTE bKeyNo,
					BYTE *szRnd,
					BYTE *szAPDU,
					BYTE bKeyHeaderLen,BYTE *szKeyHeader,
					BYTE *szPDID,
					BYTE *szEncKey,
					BYTE *szMAC)
{
	if(!validation()) return -1;
	return pcmd->cmd_1042(bVer,szAPPID,
					bKeyNo,
					szRnd,
					szAPDU,
					bKeyHeaderLen,szKeyHeader,
					szPDID,
					szEncKey,
					szMAC);
}


//4.13	计算通行卡的KeyA密钥 2011
int	__stdcall 	cmd_2011(BYTE *szCityCode,BYTE *szSerialNo,BYTE *szAuditNo,BYTE *szCardMAC,BYTE bKeyNum,BYTE *szSectorID,
				BYTE *szKey)
{
	if(!validation()) return -1;
	return pcmd->cmd_2011(szCityCode,szSerialNo,szAuditNo,szCardMAC,bKeyNum,szSectorID,szKey);
}



//4.14	OBU车辆信息文件解密 1043
int	__stdcall	cmd_1043(BYTE bVer,BYTE *szAPPID,BYTE bKeyIndex,BYTE bInLen,BYTE *szEncData,
				BYTE *bOutLen,BYTE *szData)
{
	if(!validation()) return -1;
	return pcmd->cmd_1043(bVer,szAPPID,bKeyIndex,bInLen,szEncData,bOutLen,szData);
}

//4.15	PSAM卡授权 1044
int	__stdcall	cmd_1044(BYTE *szSAMNo,BYTE *szRnd,
				DWORD dwRoadID,char *strRoadName,
				DWORD dwStationID, char *strStationName,BYTE bStationType,
				BYTE bLaneType,BYTE bLaneID,
				BYTE *bAPDULen,BYTE *szAPDU,char *strListNo)
{
	if(!validation()) return -1;
	return pcmd->cmd_1044(szSAMNo,szRnd,
				dwRoadID,strRoadName,
				dwStationID, strStationName,bStationType,
				bLaneType,bLaneID,
				bAPDULen,szAPDU,strListNo);
}

//4.16	PSAM卡在线授权确认 1045
int	__stdcall	cmd_1045(BYTE *szSAMNo,char *strListNo,
				WORD wSW1SW2,BYTE bResult)
{

	if(!validation()) return -1;
	return pcmd->cmd_1045(szSAMNo,strListNo,wSW1SW2,bResult);

}

//4.17	PSAM卡在线签到 1046
int	__stdcall	cmd_1046(BYTE *szSAMNo,BYTE *szTerminalNo,
				DWORD dwRoadID,char *strRoadName,
				DWORD dwStationID, char *strStationName,BYTE bStationType,
				BYTE bLaneType,BYTE bLaneID,
				BYTE *szTerminalTime,
				char *strListNo)
{

	if(!validation()) return -1;
	return pcmd->cmd_1046(szSAMNo,szTerminalNo,
				dwRoadID,strRoadName,
				dwStationID, strStationName,bStationType,
				bLaneType,bLaneID,
				szTerminalTime,
				strListNo);

}



