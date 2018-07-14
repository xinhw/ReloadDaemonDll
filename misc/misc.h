
#ifndef		__MISC_H__
#define		__MISC_H__

#include "..\\datatypedef.h"
#include <string>
using std::string;

class CMisc
{

public:
	CMisc();
	~CMisc();

public:
	//	Windows�йرջ������������
	static BOOL SystemShutdown(DWORD dwFlag);
	static BOOL GetDiscInfo(DWORD &dwTotalSpace,DWORD &dwFreeSpace);	
	//	���ñ���ʱ��
	static BOOL SetBoxDatetime(SYSTEMTIME *pstm);

	//	����ļ��Ƿ����
	static bool FindFileByName(TCHAR *pszFile);

	static WORD calcCRC(BYTE *data,UINT length);

	static BYTE bcd2bin(BYTE b);

 

	static BOOL GetMemoryInfo(DWORD &dwTotalSpace,DWORD &dwFreeSpace);

	static	void	Int2Bytes(int n,BYTE *pch);
	static void	Bytes2Int(BYTE *pch,int *n);
	//
	static UCHAR bcd(UCHAR ch);

	static const string toHexString(const unsigned char* input, const int datasize);
	
	static void XorBytes(int nlen,const BYTE *var1,const BYTE *var2,BYTE *var);
	
};

#endif

