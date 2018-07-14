
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
	//	Windows中关闭或者重启计算机
	static BOOL SystemShutdown(DWORD dwFlag);
	static BOOL GetDiscInfo(DWORD &dwTotalSpace,DWORD &dwFreeSpace);	
	//	设置本地时钟
	static BOOL SetBoxDatetime(SYSTEMTIME *pstm);

	//	检查文件是否存在
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

