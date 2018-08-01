/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/19 10:35:48

    Reversion:
			万集读卡器接口    
-------------------------------------------------------------------------*/
#pragma once
#include "CardReader.h"


#define SUCCESS	0
#define FAILURE 1

class CNXRsuReader : public CCardReader
{
public:
	CNXRsuReader(void);
	~CNXRsuReader(void);
private:
	HMODULE m_hDll;
	long m_hDevice;

	typedef long (WINAPI *LPFN_RSU_Open)(int mode, char *dev, int port);
	typedef int (WINAPI *LPFN_RSU_Close)(long fd);
	// RSU初始化
	typedef int (WINAPI *LPFN_RSU_INIT_rq)(long fd, char *Time, int BSTInterval, int RetryInterval, int TxPower, int PLLChannelID, int TimeOut);
	// RSU初始化返回
	typedef int (WINAPI *LPFN_RSU_INIT_rs)(long fd, int * RSUStatus, char *RSUinfo, int TimeOut);
	// PSAM通道复位
	typedef int (WINAPI *LPFN_PSAM_Reset_rq)(long fd, int PSAMSlot, long baud, int TimeOut);
	// PSAM通道复位返回
	typedef int (WINAPI *LPFN_PSAM_Reset_rs)(long fd, int PSAMSlot, char *Data, int TimeOut);
	// PSAM通道指令
	typedef int (WINAPI *LPFN_PSAM_CHANNEL_rq)(int fd, int PSAMSlot , int APDUList ,char *APDU, int TimeOut);
	// PSAM通道指令返回
	typedef int (WINAPI *LPFN_PSAM_CHANNEL_rs)(int fd, int PSAMSlot , int * APDUList ,char *Data, int TimeOut);
	// RSU信息查询
	typedef int (WINAPI *LPFN_RSU_Info_rq)(long fd, int TimeOut);
	// RSU信息查询返回
	typedef int (WINAPI *LPFN_RSU_Info_rs)(long fd, int * rlen, char *RSUinfo, int TimeOut);
	// 写日志文件
	typedef void (WINAPI *LPFN_RSUDoLog)(bool IsWriteLog);

	// BST
	typedef int (WINAPI *LPFN_INITIALISATION_rq)(long fd, char *BeaconID , char *Time, int Profile,
							int MandApplicationlist,char * MandApplication,
							int Profilelist, int TimeOut);
	// VST
	typedef int (WINAPI *LPFN_INITIALISATION_rs)(long fd, int * ReturnStatus, int * Profile,
							int * Applicationlist, char *Application,
							char *ObuConfiguration, int TimeOut );

	// 透传接口
	typedef int (WINAPI *LPFN_TransferChannel_rq)(long fd, int mode, int DID, int ChannelID,
							int APDULIST, char *APDU, int TimeOut);

	// 透传应答
	typedef int (WINAPI *LPFN_TransferChannel_rs)(long fd, int *DID, int * ChannelID, int * APDULIST,
							char *Data, int * ReturnStatus, int TimeOut);

	// OBU休眠请求
	typedef int (WINAPI *LPFN_SetMMI_rq)(long fd, int mode, int DID, int SetMMIPara, int TimeOut);
	// OBU休眠应答
	typedef int (WINAPI *LPFN_SetMMI_rs)(long fd, int * DID, int * ReturnStatus, int TimeOut);

	//	发出读取OBU文件请求(GetSecure_rq)
	typedef int (WINAPI *LPFN_GetSecure_rq)(long fd,int accessCredentialsOp ,int mode, int DID,
										char * pAccessCredentials,int keyIdForEncryptOp, int FID, int offset, int length, char * pRandRSU, 
										int KeyIdForAuthen,int KeyIdForEncrypt, int TimeOut);
	//	获取读取OBU文件请求回应(GetSecure_rs)
	typedef int (WINAPI *LPFN_GetSecure_rs)(long fd, int * DID,int * FID,int * length,char * pFile,
										char * authenticator,int * ReturnStatus, int TimeOut);


	LPFN_RSU_Open lpfn_RSU_Open;
	LPFN_RSU_Close lpfn_RSU_Close;
	LPFN_RSU_INIT_rq lpfn_RSU_INIT_rq;
	LPFN_RSU_INIT_rs lpfn_RSU_INIT_rs;
	LPFN_PSAM_Reset_rq lpfn_PSAM_Reset_rq;
	LPFN_PSAM_Reset_rs lpfn_PSAM_Reset_rs;
	LPFN_PSAM_CHANNEL_rq lpfn_PSAM_CHANNEL_rq;
	LPFN_PSAM_CHANNEL_rs lpfn_PSAM_CHANNEL_rs;
	LPFN_INITIALISATION_rq lpfn_INITIALISATION_rq;
	LPFN_INITIALISATION_rs lpfn_INITIALISATION_rs;
	LPFN_TransferChannel_rq lpfn_TransferChannel_rq;
	LPFN_TransferChannel_rs lpfn_TransferChannel_rs;
	LPFN_SetMMI_rq lpfn_SetMMI_rq;
	LPFN_SetMMI_rs lpfn_SetMMI_rs;
	LPFN_GetSecure_rq lpfn_GetSecure_rq;
	LPFN_GetSecure_rs lpfn_GetSecure_rs;


private:
	unsigned long m_ulCardNum;
	BYTE m_bPSAMNode;

public:
	virtual UINT Open(char *strAddress, unsigned int iBaud);
	virtual void Close();
	virtual UINT Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult);
	virtual UINT RunCmd(char *strCmd, char *strResult);
	virtual UINT Halt();

	//	PSAM卡函数
	virtual UINT PSAM_RunCmd(char *strCmd, char *strResult);
	virtual UINT PSAM_Atr(BYTE bNode,BYTE &brLen,char *strATR);
	virtual UINT SecureRead(BYTE bKeyIndex,BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE &bRetFileLen,char *strResp);


};
