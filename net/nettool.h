 

#ifndef		__NETTOOL_H_
#define		__NETTOOL_H_

class CNetTool
{
public:
	CNetTool();
	~CNetTool();

public:
	//	��ü��������
	static int	getComputerName(TCHAR *pszName);
	//	��ü����IP��ַ
	static int	getIP(TCHAR *pszIP);

#ifndef WINCE
	//	��ü������MAC��ַ��
	static int getMAC(TCHAR * mac);
#endif

};
#endif
