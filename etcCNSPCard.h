
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

//	����������
#define READER_TYPE_CPU_CARD		0x00
#define READER_TYPE_OBU				0x01
#define READER_TYPE_XIONGDI			0x02

typedef void  (__stdcall *CALLBACKFUNC)(int nLen,char *pszstr);
CALLBACKFUNC pMyCallback = NULL;
void callbackMessage(char *strmsg);


extern "C"
{
	/*	��������ʵ��*/
	CTcpTransfer *ptransfer=NULL;
	/*	��̨��Կ����ʵ��*/
	ClsCommand *pcmd = NULL;
	/*	������ʵ��*/
	CCardReader *preader = NULL;

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
		nType	[in]	���������ͣ�0��������𿨶�������1�����򼯶�����
		ncom	[in]	�˿ں�
		nbaud	[in]	������		����𿨶����������ǣ�115200���򼯶������ǣ�0
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
		elf15		[out]	��Ƭ���л��������ļ�
		elf16		[out]	�ֿ��˻��������ļ�
		dwRemain	[out]	���
	*/
	int __stdcall cpuReadCardFiles(BYTE *elf15,BYTE *elf16,DWORD &dwRemain);

	/*
		8.	��ȡ��¼�ļ�
		bFileID		[in]	�ļ���ʶ��SFI
		bNo			[in]	��¼��
		bLen		[in]	��¼����
		szRec		[out]	��¼

		���Զ�ȡ��0018,0017,0019�ļ�
	*/
	int __stdcall cpuReadCardRecord(BYTE bFileID,BYTE bNo,BYTE bLen,BYTE *szRec);

	/*9. �û���һ��
		szFile0015	[in]	0015�ļ�����
	*/
	int __stdcall cpuInit(BYTE *szFile0015);


	/*10. ���³ֿ��˻��������ļ�
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szFile		[in]	�ֿ��˻��������ļ�		
	*/
	int __stdcall cpuUpdateUserFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile);


	/*11. ���¿����л��������ļ�
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szFile		[in]	�����л��������ļ�		
	*/
	int __stdcall cpuUpdateIssueFile(BYTE bVer,BYTE *szAPPID,BYTE *szFile);

	/*12. ���¿�Ƭ��Ч��
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szValidDate	[in]	�µ���Ч��	
	*/
	int __stdcall cpuUpdateValidDate(BYTE bVer,BYTE *szAPPID,BYTE *szValidDate);

	/*13. Ȧ��
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		dwAmount	[in]	Ȧ����
		szDateTime	[in]	��������ʱ��(BCD)
		wSeqNo		[out]	���߽������
		szTAC		[out]	TAC
	*/
	int __stdcall cpuCredit(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,BYTE *szDateTime,BYTE *szDeviceNo,
							WORD &wSeqNo,BYTE *szTAC);

	/*14. ���±���000E�ļ�
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		szFile000E	[in]	000E�ļ�����
	*/
	int __stdcall cpuUpdateFile000E(BYTE bVer,BYTE *szAPPID,BYTE *szFile000E);

	/*15. ����
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		dwAmount	[in]	���ѽ��
		dwAuditNo	[in]	�ն˽������
		szDateTime	[in]	��������ʱ��(BCD)
		wSeqNo		[out]	�ѻ��������
		szTAC		[out]	TAC
	*/
	int __stdcall cpuPurchase(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,BYTE *szDeviceNo,
							WORD &wSeqNo,BYTE *szTAC);

	/*16. CPU����֤TAC
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		dwAmount	[in]	���ѽ��
		bTransFlag	[in]	�������ͱ�ʶ
		szDeviceNo	[in]	�ն˺�
		dwAuditNo	[in]	�ն˽������
		szDateTime	[in]	��������ʱ��(BCD)
		szTAC		[in]	TAC
	*/
	int __stdcall cpuVerifyTAC(BYTE bVer,BYTE *szAPPID,
							   DWORD dwAmount,BYTE bTransFlag,BYTE *szDeviceNo,DWORD dwAuditNo,BYTE *szDateTime,
							   BYTE *szTAC);
	/*
	17. CPU����װPIN
		bVer		[in]	��Ƭ�汾��
		szAPPID		[in]	��ƬӦ�����к�
		bPINLen		[in]	PIN�ĳ���
		szPIN		[in]	�µ�PIN
	*/
	int __stdcall cpuReloadPIN(BYTE bVer,BYTE *szAPPID,BYTE bPINLen,BYTE *szPIN);


	/***************************************************************************************/
	/*				OBU����	    														   */
	/***************************************************************************************/
	/*18. OBUһ��
		elf01_mk	[in]	ϵͳ��Ϣ�ļ�
		elf01_adf01	[in]	������Ϣ�ļ�
	*/
	int __stdcall obuInit(BYTE *elf01_mk,BYTE *elf01_adf01);
	int __stdcall obuPreInit(BYTE *elf01_mk);
	int	__stdcall obuGetUID(BYTE *szUID);

	/*19. OBU��ȡ ϵͳ��Ϣ�ļ���Ϣ
		elf01_mk	[in]	ϵͳ��Ϣ�ļ�
	*/
	int __stdcall obuRead(BYTE *elf01_mk);
	/*	OBU��ȡ ������Ϣ�ļ�*/
	int __stdcall obuReadVehicleFile(BYTE bNode,BYTE bVer,BYTE *szPlainFile);

	/*20. OBU�����ļ�
		bVer	[in]	OBU��ͬ�汾��
		szAPPID	[in]	OBU��ͬ���к�
		bFileType	[in]	�ļ����ͣ�0x01: ϵͳ��Ϣ�ļ���0x02: ������Ϣ�ļ���
		szFile	[out]	�ļ���Ϣ
	*/
	int __stdcall obuUpdateFile(BYTE bVer,BYTE *szAPPID,BYTE bFileType,BYTE *szFile);

	/*21. OBU��ж��־�޸�
		bVer	[in]	OBU��ͬ�汾��
		szAPPID	[in]	OBU��ͬ���к�
		bFlag	[in]	OBU��ж��־
	*/
	int __stdcall obuUpdateLoadFlag(BYTE bVer,BYTE *szAPPID,BYTE bFlag);


	bool validation(int nlevel);

};