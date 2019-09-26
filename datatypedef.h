
#ifndef		__DATATYPEDEF_H_
#define		__DATATYPEDEF_H_

	//	等待socket的模式
	#define		MODE_SEND		0
	#define		MODE_RECV		1

	//	包的大小
	#define		MIN_PACKAGE_SIZE		0x100
	#define		MAX_PACKAGE_SIZE		1500

	#define		HUGE_BUFFER_SIZE		8192

	#ifndef BYTE
	#define BYTE unsigned char
	#endif
	#ifndef UINT
	#define UINT unsigned int
	#endif


	//	充值机connect后的线程参数
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


	//2、	报文头结构如下：
	//1	报文特征码	MessageNote	C	4
	//2	消息类型	TranType	US	2
	//3	代理商编码	BankCode	C	6
	//4	请求方流水号	ReqLogId	L	4
	//5	数据长度	DataLen	UL	4
	//6	报文请求时间	TransTime	C	14
	//7	押码	MAC	L	4
	//8	压缩标志	CompressFlag	UT	1
	//9	保留	Reserve	C	1
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

	/*	线程参数*/
	typedef	struct	threadParam
	{
		int		nportno;
		int		ntimeout;
	}THREADPARAM;
	#define PTHREADPARAM THREADPARAM *


	typedef	struct	tagcardkey
	{
		BYTE bIndex;				/*	密钥索引*/
		BYTE bHeaderLen;			/*	密钥头长度*/
		BYTE *szKeyHeader;			/*	密钥头*/
		char *strDesc;				/*	密钥描述*/
		BYTE *P1P2;					/*	P1和P2参数*/
	}CARDKEY;
	#define PCARDKEY CARDKEY *


	typedef struct tagwritefilerec
	{
		char *p1;					/*	写记录文件 P1*/
		char *p2;					/*	写记录文件 P2*/
		BYTE bLen;					/*	写记录文件 的长度*/
		char *applabel;				/*	复合应用标志（长度为6）*/
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