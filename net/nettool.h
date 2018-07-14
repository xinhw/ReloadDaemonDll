 

#ifndef		__NETTOOL_H_
#define		__NETTOOL_H_

class CNetTool
{
public:
	CNetTool();
	~CNetTool();

public:
	//	获得计算机名称
	static int	getComputerName(TCHAR *pszName);
	//	获得计算机IP地址
	static int	getIP(TCHAR *pszIP);

#ifndef WINCE
	//	获得计算机的MAC地址；
	static int getMAC(TCHAR * mac);
#endif

};
#endif
