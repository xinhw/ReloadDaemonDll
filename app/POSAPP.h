
#ifndef		__POSAPP_H_
#define		__POSAPP_H_


class ClsReload
{
public:
	ClsReload();
	~ClsReload();
	
public:
	int			process_request(  PPACKAGEHEADER ph,const WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	

private:

	//3.1	����Աǩ����֤1031��һ�ڲ�ʵ�֣�
	int		cmd_1031(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.2	������֤1032
	int		cmd_1032(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.3	CPU�����η�����֤1033
	int		cmd_1033(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.4	OBU���η�����֤1034
	int		cmd_1034(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.5	�޸Ĵ�ֵ����Ч����֤1035
	int		cmd_1035(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.6	������֤ 1036
	int		cmd_1036(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.7	����TAC��֤ 1037
	int		cmd_1037(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.8	��ж��־�޸�1038
	int		cmd_1038(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.9	PIN������֤1039
	int		cmd_1039(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.10	CPU����Կ��ȡ1040
	int		cmd_1040(PPACKAGEHEADER pph ,WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.11	CPU�����η��в����ļ���֤1041
	int		cmd_1041(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	//3.12	����ͨ�п���KeyA��Կ 2011
	int		cmd_2011(WORD wLenIn,BYTE *pszData,WORD &wSize,BYTE *pszResult);
	CString   retMessage( int  i);

private:
	WORD			wErrorCode;
	DWORD			nTransNo;
	PACKAGEHEADER	m_ph;


private:
	char	m_strBankID[20];		//	��������Ψһ���
	BYTE	m_pszAPPID[8];			//	�û�������orOBU���ţ�
	WORD	m_wRemainCount;			//	��Ա��豸��ʣ�������


};
#endif

