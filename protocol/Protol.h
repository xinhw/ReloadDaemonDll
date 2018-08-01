/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/31 16:06:40

    Reversion:
        
-------------------------------------------------------------------------*/
 
//PROTOL.H

#ifndef _PROTOL_H
#define _PROTOL_H


class CProtocol
{

public:
	CProtocol();
	~CProtocol();

public:
	#define MIN_DATAGRAM_SIZE		40


	//	�������Կͻ��˵�����
	int		parse_a_block(WORD nSize,BYTE *pszBuf,
						PPACKAGEHEADER pph,
						WORD &wLen,BYTE *pszData);
	
	//	���ͻ��˵�Ӧ�����
	int		combine_a_block(PPACKAGEHEADER pph,WORD wLen,BYTE *pszData,
						WORD	&wSize,BYTE	*pszBufOut);


	void	print_package_header(PPACKAGEHEADER pph);
};

#endif //_PROTOL_H