// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ETCCNSPCARD_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ETCCNSPCARD_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef ETCCNSPCARD_EXPORTS
#define ETCCNSPCARD_API __declspec(dllexport)
#else
#define ETCCNSPCARD_API __declspec(dllimport)
#endif

#ifdef DEBUG_TEST
#pragma message("测试程序")
#else
#pragma message("动态库")
#endif

#define MAX_READER_NUM	32

/*

	V0.17	增加了多串口支持
	V0.18	增加前置通信超时设置
			删除TcpTransfer类中的send_recv函数
	V0.19	增加PSAM卡签到、PSAM卡授权确认的接口
			1045,1046报文
*/
#define DLL_VERSION		"V0.19 多串口国密算法动态库"

//	读卡器类型
#define READER_TYPE_CPU_CARD		0x00			//	航天金卡用户卡读卡器
#define READER_TYPE_OBU				0x01			//	万集OBU读卡器
#define READER_TYPE_XIONGDI			0x02			//	雄帝用户卡读卡器
#define READER_TYPE_JINYI			0x03			//	金溢OBU读卡器


int gnDefaultCom = 0;


/*回调函数*/
typedef void  (__stdcall *CALLBACKFUNC)(int nLen,char *pszstr);
CALLBACKFUNC pMyCallback = NULL;
void callbackMessage(char *strmsg);

/*	网络连接实例*/
CTcpTransfer *ptransfer[MAX_READER_NUM];
/*	后台密钥服务实例*/
ClsCommand *pcmd[MAX_READER_NUM];
/*	读卡器实例*/
CCardReader *preader[MAX_READER_NUM];

bool validation(int nlevel,int ncom = gnDefaultCom);


extern "C"
{
	//	返回动态库版本
	void	__stdcall	getDllVersion(char *strVer);

	/*	设置回调函数：因为一发的时间比较长，中间可通过回调函数返回信息，刷新界面；也可以通过回调知道进展*/
	void	__stdcall	setCallbackFunc(CALLBACKFUNC p);

	/***************************************************************************************/
	/*				通用函数															   */
	/***************************************************************************************/
	/*1. 连接到在线密钥服务前置： Online Keys Service
		strip	[in]	前置的IP地址
		wport	[in]	前置的PORT
	*/
	int __stdcall connectOKS(char *strip,WORD wport,int ncom = gnDefaultCom);

	/*2. 关闭到在线密钥服务前置的连接*/
	int __stdcall disconnectOKS(int ncom = gnDefaultCom);

	/*3. 打开读卡器
		nType	[in]	读卡器类型，0——航天金卡读卡器，1——万集读卡器
		ncom	[in]	端口号
		nbaud	[in]	波特率		航天金卡读卡器参数是：115200，万集读卡器是：0
	*/
	int __stdcall openReader(int nType,int ncom,int nbaud);

	/*4. 关闭读卡器*/
	int __stdcall closeReader(int ncom = gnDefaultCom);

	/*5. 操作员签到认证
		strOperator	[in]	操作员卡号
	*/
	int __stdcall signIn(char *strOperator,int ncom = gnDefaultCom);

	/***************************************************************************************/
	/*				CPU函数																   */
	/***************************************************************************************/
	/*6. CPU卡复位
		szSNO	[in]	卡唯一号
		bATSLen	[in]	卡片ATS返回的长度
		szATS	[in]	卡片ATS信息
	*/
	int __stdcall cpuATS(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS,int ncom = gnDefaultCom);

	/*7. 读CPU卡文件
		elf15		[out]	卡片发行基本数据文件
		elf16		[out]	持卡人基本数据文件
		dwRemain	[out]	余额
	*/
	int __stdcall cpuReadCardFiles(BYTE *elf15,BYTE *elf16,DWORD &dwRemain,int ncom = gnDefaultCom);
	/*
	读取ADF下的二进制文件
		bFileID		[in]	短文件标识符
		bOffset		[in]	位移
		bLength		[in]	读取长度
		szFile		[out]	读取的信息
	*/
	int __stdcall cpuReadAdfFile(BYTE bFileID,BYTE bOffset,BYTE bLength,BYTE *szFile,int ncom = gnDefaultCom);
	/*
		8.	读取记录文件
		bFileID		[in]	文件标识，SFI
		bNo			[in]	记录号
		bLen		[in]	记录长度
		szRec		[out]	记录

		可以读取：0018,0017,0019文件
	*/
	int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec,int ncom = gnDefaultCom);

	/*9. 用户卡一发
		szFile0015	[in]	0015文件内容
	*/
	int __stdcall cpuInit(BYTE *szFile0015,int ncom = gnDefaultCom);


	/*10. 更新持卡人基本数据文件
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szFile		[in]	持卡人基本数据文件		
	*/
	int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile,int ncom = gnDefaultCom);


	/*11. 更新卡发行基本数据文件
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szFile		[in]	卡发行基本数据文件		
	*/
	int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile,int ncom = gnDefaultCom);

	/*12. 更新卡片有效期
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szValidDate	[in]	新的有效期	
	*/
	int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate,int ncom = gnDefaultCom);

	/*13. 圈存
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		dwAmount	[in]	圈存金额
		szDateTime	[in]	交易日期时间(BCD)
		wSeqNo		[out]	在线交易序号
		szTAC		[out]	TAC
	*/
	int __stdcall cpuCredit(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,BYTE *szDateTime,BYTE *szDeviceNo,
							WORD &wSeqNo,BYTE *szTAC,int ncom = gnDefaultCom);

	/*14. 更新本地000E文件
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szFile000E	[in]	000E文件内容
	*/
	int __stdcall cpuUpdateFile000E(BYTE bVer,BYTE *szAPPID,BYTE *szFile000E,int ncom = gnDefaultCom);

	/*15. 消费
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		dwAmount	[in]	消费金额
		dwAuditNo	[in]	终端交易序号
		szDateTime	[in]	交易日期时间(BCD)
		wSeqNo		[out]	脱机交易序号
		szTAC		[out]	TAC
	*/
	int __stdcall cpuPurchase(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,BYTE *szDeviceNo,
							WORD &wSeqNo,BYTE *szTAC,int ncom = gnDefaultCom);

	/*16. CPU卡验证TAC
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		dwAmount	[in]	消费金额
		bTransFlag	[in]	交易类型标识
		szDeviceNo	[in]	终端号
		dwAuditNo	[in]	终端交易序号
		szDateTime	[in]	交易日期时间(BCD)
		szTAC		[in]	TAC
	*/
	int __stdcall cpuVerifyTAC(BYTE bVer,BYTE *szAPPID,
							   DWORD dwAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD dwAuditNo,BYTE *szDateTime,
							   BYTE *szTAC,int ncom = gnDefaultCom);
	/*
	17. CPU卡重装PIN
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		bPINLen		[in]	PIN的长度
		szPIN		[in]	新的PIN
	*/
	int __stdcall cpuReloadPIN(BYTE bVer,BYTE *szAPPID,BYTE bPINLen,BYTE *szPIN,int ncom = gnDefaultCom);


	/***************************************************************************************/
	/*				OBU函数	    														   */
	/***************************************************************************************/
	/*18. OBU一发
		elf01_mk	[in]	系统信息文件
		elf01_adf01	[in]	车辆信息文件
	*/
	int __stdcall obuInit(BYTE *elf01_mk,BYTE *elf01_adf01,int ncom = gnDefaultCom);
	int __stdcall obuPreInit(WORD wDFID,BYTE *elf01_mk,int ncom = gnDefaultCom);
	int	__stdcall obuGetUID(BYTE *szUID,int ncom = gnDefaultCom);

	/*19. OBU读取 系统信息文件信息
		elf01_mk	[in]	系统信息文件
	*/
	int __stdcall obuRead(BYTE *elf01_mk,int ncom = gnDefaultCom);
	/*	OBU读取 车辆信息文件*/
	int __stdcall obuReadVehicleFile(BYTE bNode,BYTE bVer,BYTE *szPlainFile,int ncom = gnDefaultCom);
	
	/*20. OBU更新文件
		bVer	[in]	OBU合同版本号
		szAPPID	[in]	OBU合同序列号
		bFileType	[in]	文件类型（0x01: 系统信息文件，0x02: 车辆信息文件）
		szFile	[out]	文件信息
	*/
	int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bFileType,BYTE *szFile,int ncom = gnDefaultCom);

	/*21. OBU拆卸标志修改
		bVer	[in]	OBU合同版本号
		szAPPID	[in]	OBU合同序列号
		bFlag	[in]	OBU拆卸标志
	*/
	int __stdcall obuUpdateLoadFlag(BYTE bVer,BYTE *szAPPID,BYTE bFlag,int ncom = gnDefaultCom);
	int __stdcall obuUnlockApplication(BYTE bVer,BYTE *szAPPID,int ncom = gnDefaultCom);

	/*22. 设置和获取前置通信超时
		dwTimeout:	超时时间（秒）
	*/
	void __stdcall setTimeout(DWORD dwTimeout,int ncom = gnDefaultCom);
	DWORD __stdcall getTimeout(int ncom = gnDefaultCom);

	/*23. 车辆信息在线解密
		dwTimeout:	超时时间（秒）
	*/
	int __stdcall obuOnlineDecodePlate(BYTE bVer,BYTE *szAPPID,
										BYTE bKeyIndex,
										BYTE bLenIn,BYTE *szEncData,
										BYTE *bLenOut,BYTE *szData,
										int ncom = gnDefaultCom);
	/*24. PSAM卡在线授权申请*/
	int __stdcall psamOnlineAuth(BYTE *szSAMNo,BYTE *szRnd,
								DWORD dwRoadID,char *strRoadName,
								DWORD dwStationID, char *strStationName,BYTE bStationType,
								BYTE bLaneType,BYTE bLaneID,
								BYTE *bAPDULen,BYTE *szAPDU,
								char *strListNo,
								int ncom = gnDefaultCom);
	/*25. PSAM卡在线签到*/
	int __stdcall psamOnlineSignIn(BYTE *szSAMNo,BYTE *szTerminalNo,
								DWORD dwRoadID,char *strRoadName,
								DWORD dwStationID, char *strStationName,BYTE bStationType,
								BYTE bLaneType,BYTE bLaneID,
								BYTE *szTerminalTime,
								int ncom = gnDefaultCom);
	/*26. PSAM卡在线授权确认*/
	int	__stdcall psamOnlineAuthConfirm(BYTE *szSAMNo,char *strListNo,
								WORD wSW1SW2,BYTE bResult,
								int ncom = gnDefaultCom);


};