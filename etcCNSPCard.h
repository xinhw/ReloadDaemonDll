
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
#pragma message("���Գ���")
#else
#pragma message("��̬��")
#endif


#define READER_TYPE_CPU_CARD		0x00
#define READER_TYPE_OBU				0x01

BYTE gszDeviceNo[6];

typedef void  (__stdcall *CALLBACKFUNC)(int nLen,char *pszstr);
CALLBACKFUNC pMyCallback = NULL;

extern "C"
{
	/*	��������ʵ��*/
	CTcpTransfer *ptransfer=NULL;
	/*	��̨��Կ����ʵ��*/
	ClsCommand *pcmd = NULL;
	/*	������ʵ��*/
	CCardReader *preader = NULL;

	/*	�����ն˻����*/
	void __stdcall setDeviceNo(BYTE *szNo);
	/*	��ȡ�ն˻����*/
	void __stdcall getDeviceNo(BYTE *szNo);

	/*	���ûص���������Ϊһ����ʱ��Ƚϳ����м��ͨ���ص�����������Ϣ��ˢ�½��棻Ҳ����ͨ���ص�֪����չ*/
	void	__stdcall	setCallbackFunc(CALLBACKFUNC p);

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

	/*3. �򿪶�����
		nType	[in]	���������ͣ�֧�ֶ������Ԥ����
		ncom	[in]	�˿ں�
		nbaud	[in]	������
	*/
	int __stdcall openReader(int nType,int ncom,int nbaud);

	/*4. �رն�����*/
	int __stdcall closeReader();

	/*5. ����Աǩ����֤
		strOperator	[in]	����Ա����
	*/
	int __stdcall signIn(char *strOperator);


	/***************************************************************************************/
	/*				CPU����																   */
	/***************************************************************************************/
	/*6. CPU����λ
		szSNO	[in]	��Ψһ��
		bATSLen	[in]	��ƬATS���صĳ���
		szATS	[in]	��ƬATS��Ϣ
	*/
	int __stdcall cpuATS(BYTE *szSNO,BYTE &bATSLen,BYTE *szATS);


	/*7. ��CPU���ļ�
		wFileID	[in]	�ļ���SFI
		bLen	[out]	�ļ�����
		szFile	[out]	�ļ�����
	*/
	int __stdcall cpuReadCardFiles(BYTE *elf15,BYTE *elf16,DWORD &dwRemain);
	int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec);

	/*8. CPU��һ��
		szFile0015	[in]	0015�ļ�����
	*/
	int __stdcall cpuInit(BYTE *szFile0015);


	/*9. ���³ֿ��˻��������ļ�
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szFile		[in]	�ֿ��˻��������ļ�		
	*/
	int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile);


	/*10. ���¿����л��������ļ�
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szFile		[in]	�����л��������ļ�		
	*/
	int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile);

	/*11. ���¿�Ƭ��Ч��
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szValidDate	[in]	�µ���Ч��	
	*/
	int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate);

	/*12. Ȧ��
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		dwAmount	[in]	Ȧ����
		szDateTime	[in]	��������ʱ��(BCD)
		wSeqNo		[out]	���߽������
		szTAC		[out]	TAC
	*/
	int __stdcall cpuCredit(BYTE bCardVer,BYTE *szAPPID,DWORD dwAmount,BYTE *szDateTime,
							WORD &wSeqNo,BYTE *szTAC);

	/*13. ����
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		dwAmount	[in]	���ѽ��
		dwAuditNo	[in]	�ն˽������
		szDateTime	[in]	��������ʱ��(BCD)
		wSeqNo		[out]	�ѻ��������
		szTAC		[out]	TAC
	*/
	int __stdcall cpuPurchase(BYTE bCardVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,
							WORD &wSeqNo,BYTE *szTAC);

	/*13. CPU����֤TAC
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		dwAmount	[in]	���ѽ��
		bTransFlag	[in]	�������ͱ�ʶ
		dwAuditNo	[in]	�ն˽������
		szDateTime	[in]	��������ʱ��(BCD)
		szTAC		[in]	TAC
	*/
	int __stdcall cpuVerifyTAC(BYTE bCardVer,BYTE *szAPPID,
							   DWORD dwAmount,BYTE bTransFlag,DWORD dwAuditNo,BYTE *szDateTime,
							   BYTE *szTAC);
	/*
	14. CPU����װ��Կ
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
	*/
	int __stdcall cpuReloadPIN(BYTE bCardVer,BYTE *szAPPID);


	/***************************************************************************************/
	/*				OBU����	    														   */
	/***************************************************************************************/
	/*6. OBU��λ
		bATRLen	[in]	OBU��ATR���صĳ���
		szATR	[in]	OBU��ATR��Ϣ
	*/
	int __stdcall obuATR(BYTE &bATRLen,BYTE *szATR);

	/*8. OBUһ��
		szEF01	[in]	������Ϣ�ļ�
	*/
	int __stdcall obuInit(BYTE *elf01_mk,BYTE *elf01_adf01);

	/*8. OBU��ȡ��Ϣ
		bVer	[in]	OBU��ͬ�汾��
		szAPPID	[in]	OBU��ͬ���к�
		wFileID	[in]	�ļ�ID
		bLen	[out]	�ļ�����
		szFile	[out]	�ļ���Ϣ
	*/
	int __stdcall obuRead(BYTE *elf01_mk,BYTE *elf01_adf01);


	/*8. OBU�����ļ�
		bVer	[in]	OBU��ͬ�汾��
		szAPPID	[in]	OBU��ͬ���к�
		bLen	[in]	�ļ�����
		szFile	[out]	�ļ���Ϣ
	*/
	int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bLen,BYTE *szFile);

};