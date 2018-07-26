
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


#define READER_TYPE_CPU_CARD		0x00
#define READER_TYPE_OBU				0x01

BYTE gszDeviceNo[6];

typedef void  (__stdcall *CALLBACKFUNC)(int nLen,char *pszstr);
CALLBACKFUNC pMyCallback = NULL;

extern "C"
{
	/*	网络连接实例*/
	CTcpTransfer *ptransfer=NULL;
	/*	后台密钥服务实例*/
	ClsCommand *pcmd = NULL;
	/*	读卡器实例*/
	CCardReader *preader = NULL;

	/*	设置终端机编号*/
	void __stdcall setDeviceNo(BYTE *szNo);
	/*	获取终端机编号*/
	void __stdcall getDeviceNo(BYTE *szNo);

	/*	设置回调函数：因为一发的时间比较长，中间可通过回调函数返回信息，刷新界面；也可以通过回调知道进展*/
	void	__stdcall	setCallbackFunc(CALLBACKFUNC p);

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

	/*3. 打开读卡器
		nType	[in]	读卡器类型（支持多读卡器预留）
		ncom	[in]	端口号
		nbaud	[in]	波特率
	*/
	int __stdcall openReader(int nType,int ncom,int nbaud);

	/*4. 关闭读卡器*/
	int __stdcall closeReader();

	/*5. 操作员签到认证
		strOperator	[in]	操作员卡号
	*/
	int __stdcall signIn(char *strOperator);


	/***************************************************************************************/
	/*				CPU函数																   */
	/***************************************************************************************/
	/*6. CPU卡复位
		szSNO	[in]	卡唯一号
		bATSLen	[in]	卡片ATS返回的长度
		szATS	[in]	卡片ATS信息
	*/
	int __stdcall cpuATS(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS);


	/*7. 读CPU卡文件
		wFileID	[in]	文件的SFI
		bLen	[out]	文件长度
		szFile	[out]	文件内容
	*/
	int __stdcall cpuReadCardFiles(BYTE *elf15,BYTE *elf16,DWORD &dwRemain);
	int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec);

	/*8. CPU卡一发
		szFile0015	[in]	0015文件内容
	*/
	int __stdcall cpuInit(BYTE *szFile0015);


	/*9. 更新持卡人基本数据文件
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szFile		[in]	持卡人基本数据文件		
	*/
	int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile);


	/*10. 更新卡发行基本数据文件
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szFile		[in]	卡发行基本数据文件		
	*/
	int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile);

	/*11. 更新卡片有效期
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		szValidDate	[in]	新的有效期	
	*/
	int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate);

	/*12. 圈存
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		dwAmount	[in]	圈存金额
		szDateTime	[in]	交易日期时间(BCD)
		wSeqNo		[out]	在线交易序号
		szTAC		[out]	TAC
	*/
	int __stdcall cpuCredit(BYTE bCardVer,BYTE *szAPPID,DWORD dwAmount,BYTE *szDateTime,
							WORD &wSeqNo,BYTE *szTAC);

	/*13. 消费
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		dwAmount	[in]	消费金额
		dwAuditNo	[in]	终端交易序号
		szDateTime	[in]	交易日期时间(BCD)
		wSeqNo		[out]	脱机交易序号
		szTAC		[out]	TAC
	*/
	int __stdcall cpuPurchase(BYTE bCardVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,
							WORD &wSeqNo,BYTE *szTAC);

	/*13. CPU卡验证TAC
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
		dwAmount	[in]	消费金额
		bTransFlag	[in]	交易类型标识
		dwAuditNo	[in]	终端交易序号
		szDateTime	[in]	交易日期时间(BCD)
		szTAC		[in]	TAC
	*/
	int __stdcall cpuVerifyTAC(BYTE bCardVer,BYTE *szAPPID,
							   DWORD dwAmount,BYTE bTransFlag,DWORD dwAuditNo,BYTE *szDateTime,
							   BYTE *szTAC);
	/*
	14. CPU卡重装密钥
		bVer		[in]	卡片版本号
		szAPPID		[in]	卡片应用序列号
	*/
	int __stdcall cpuReloadPIN(BYTE bCardVer,BYTE *szAPPID);


	/***************************************************************************************/
	/*				OBU函数	    														   */
	/***************************************************************************************/
	/*6. OBU复位
		bATRLen	[in]	OBU的ATR返回的长度
		szATR	[in]	OBU的ATR信息
	*/
	int __stdcall obuATR(BYTE &bATRLen,BYTE *szATR);

	/*8. OBU一发
		szEF01	[in]	车辆信息文件
	*/
	int __stdcall obuInit(BYTE *elf01_mk,BYTE *elf01_adf01);

	/*8. OBU读取信息
		bVer	[in]	OBU合同版本号
		szAPPID	[in]	OBU合同序列号
		wFileID	[in]	文件ID
		bLen	[out]	文件长度
		szFile	[out]	文件信息
	*/
	int __stdcall obuRead(BYTE *elf01_mk,BYTE *elf01_adf01);


	/*8. OBU更新文件
		bVer	[in]	OBU合同版本号
		szAPPID	[in]	OBU合同序列号
		bLen	[in]	文件长度
		szFile	[out]	文件信息
	*/
	int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bLen,BYTE *szFile);

};