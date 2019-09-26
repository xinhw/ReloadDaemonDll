
#ifndef		__DATATYPEDEF_H_
#define		__DATATYPEDEF_H_

	//	�ȴ�socket��ģʽ
	#define		MODE_SEND		0
	#define		MODE_RECV		1

	//	���Ĵ�С
	#define		MIN_PACKAGE_SIZE		0x100
	#define		MAX_PACKAGE_SIZE		1500

	#define		HUGE_BUFFER_SIZE		8192

	#ifndef BYTE
	#define BYTE unsigned char
	#endif
	#ifndef UINT
	#define UINT unsigned int
	#endif


	//	��ֵ��connect����̲߳���
	typedef	struct tagClientSock
	{
		SOCKET			nsocket;
		SOCKADDR_IN		skaddr; 
		
		struct sockaddr udpaddr;
		UINT			nLen;
		BYTE			pszBuf[MAX_PACKAGE_SIZE];

		int				nWaitTimeOut;
	}CLIENTSOCK;
	#define PCLIENTSOCK CLIENTSOCK *


	//2��	����ͷ�ṹ���£�
	//1	����������	MessageNote	C	4
	//2	��Ϣ����	TranType	US	2
	//3	�����̱���	BankCode	C	6
	//4	������ˮ��	ReqLogId	L	4
	//5	���ݳ���	DataLen	UL	4
	//6	��������ʱ��	TransTime	C	14
	//7	Ѻ��	MAC	L	4
	//8	ѹ����־	CompressFlag	UT	1
	//9	����	Reserve	C	1
	typedef	struct	tagPackageHeader
	{
		char	strMessageNote[5];
		WORD	wTransType;
		char	strBankCode[7];
		DWORD	dwReqLogId;
		DWORD	dwDataLen;
		char	strTransTime[15];
		BYTE	pszMAC[4];
		BYTE	bCompressFlag;
		BYTE	bRFU;
	}PACKAGEHEADER;

	#define PPACKAGEHEADER	PACKAGEHEADER *

	#define TOM(byteValue) (byteValue/1024/1024)
	#define TOK(byteValue) (byteValue/1024)
	#define	TOG(byteValue) (byteValue/1024/1024/1024)
	#define  NHEADDATALEN 40

	/*	�̲߳���*/
	typedef	struct	threadParam
	{
		int		nportno;
		int		ntimeout;
	}THREADPARAM;
	#define PTHREADPARAM THREADPARAM *


	typedef	struct	tagcardkey
	{
		BYTE bIndex;				/*	��Կ����*/
		BYTE bHeaderLen;			/*	��Կͷ����*/
		BYTE *szKeyHeader;			/*	��Կͷ*/
		char *strDesc;				/*	��Կ����*/
		BYTE *P1P2;					/*	P1��P2����*/
	}CARDKEY;
	#define PCARDKEY CARDKEY *


	typedef struct tagwritefilerec
	{
		char *p1;					/*	д��¼�ļ� P1*/
		char *p2;					/*	д��¼�ļ� P2*/
		BYTE bLen;					/*	д��¼�ļ� �ĳ���*/
		char *applabel;				/*	����Ӧ�ñ�־������Ϊ6��*/
	}WRITEFILEREC;
	#define PWRITEFILEREC WRITEFILEREC *


	typedef struct tagOBUKey
	{
		char *strAPDU;
		char *strKeyHeader;
		char *strPID;
		char *strDID;
		char *strDesc;
	}OBUKEY;

#endif