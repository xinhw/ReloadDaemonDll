 



CProtocol::CProtocol()
{
}


CProtocol::~CProtocol()
{
}



int		CProtocol::parse_a_block(WORD nSize,BYTE *pszBuf,
					  PPACKAGEHEADER pph,
					  WORD &wLen,BYTE *pszData)
{
	WORD	nLen;

	if(nSize<MIN_DATAGRAM_SIZE) return ERR_DATAGRAM;
	if(pph==NULL) return ERR_PARAM;


	memset(pph,0x00,sizeof(PACKAGEHEADER));

	nLen = 0;
	//1	报文特征码		MessageNote		C	4
	memcpy(pph->strMessageNote,pszBuf+nLen,4);
	nLen = nLen + 4;

	//2	消息类型		TranType		US	2
	pph->wTransType = pszBuf[nLen];
	pph->wTransType = pph->wTransType*0x100 + pszBuf[nLen+1];
	nLen = nLen + 2;

	//3	代理商编码		BankCode		C	6
	memcpy(pph->strBankCode,pszBuf+nLen,6);
	nLen = nLen + 6;
	//4	请求方流水号	ReqLogId		L	4
	CMisc::Bytes2Int(pszBuf+nLen,(int *)&pph->dwReqLogId);
	nLen = nLen + 4;
	//5	数据长度		DataLen			UL	4
	CMisc::Bytes2Int(pszBuf+nLen,(int *)&pph->dwDataLen);
	nLen = nLen + 4;
	//6	报文请求时间	TransTime		C	14
	memcpy(pph->strTransTime,pszBuf+nLen,14);
	nLen = nLen + 14;
	//7	押码			MAC				L	4
	memcpy(pph->pszMAC,pszBuf+nLen,4);
	nLen = nLen + 4;	
	//8	压缩标志		CompressFlag	UT	1
	pph->bCompressFlag = pszBuf[nLen];
	nLen++;
	//9	保留			Reserve			C	1
	pph->bRFU = pszBuf[nLen];
	nLen++;
		
	wLen = nSize-nLen;
	memcpy(pszData,pszBuf+nLen,wLen);
	
	return 0;	
}


int		CProtocol::combine_a_block(PPACKAGEHEADER pph,WORD wLen,BYTE *pszData,
						WORD	&wSize,BYTE	*pszBufOut)
{
	WORD		nLen;

	if(pph==NULL) return ERR_PARAM;
	
	nLen = 0;
	//1	报文特征码		MessageNote		C	4
	memcpy(pszBufOut,pph->strMessageNote,4);
	nLen = nLen + 4;
	//2	消息类型		TranType		US	2
	pszBufOut[nLen] = (BYTE)(pph->wTransType>>8);
	pszBufOut[nLen+1] = (BYTE)(pph->wTransType&0xff);
	nLen = nLen + 2;
	//3	代理商编码		BankCode		C	6
	memcpy(pszBufOut+nLen,pph->strBankCode,6);
	nLen = nLen + 6;
	//4	请求方流水号	ReqLogId		L	4
	CMisc::Int2Bytes(pph->dwReqLogId,pszBufOut+nLen);
	nLen = nLen + 4;
	//5	数据长度		DataLen			UL	4
	CMisc::Int2Bytes(wLen,pszBufOut+nLen);
	nLen = nLen + 4;
	//6	报文请求时间	TransTime		C	14
	memcpy(pszBufOut+nLen,pph->strTransTime,14);
	nLen = nLen + 14;
	//7	押码			MAC				L	4
	memcpy(pszBufOut+nLen,pph->pszMAC,4);
	nLen = nLen + 4;
	//8	压缩标志		CompressFlag	UT	1
	pszBufOut[nLen] = pph->bCompressFlag;
	nLen = nLen + 1;
	//9	保留			Reserve			C	1
	pszBufOut[nLen] = pph->bRFU;
	nLen = nLen + 1;

	memcpy(pszBufOut+nLen,pszData,wLen);
	wSize = wLen + nLen;

	return 0;
}





void	CProtocol::print_package_header(PPACKAGEHEADER pph)
{
	PRINTK("\n消息类型:		%04x",pph->wTransType);
	PRINTK("\n报文特征码:	%s",pph->strMessageNote);
	PRINTK("\n代理商编码:	%s",pph->strBankCode);
	PRINTK("\n请求方流水号:	%u",pph->dwReqLogId);
	PRINTK("\n数据长度:		%u",pph->dwDataLen);
	PRINTK("\n报文请求时间:	%s",pph->strTransTime);
	PRINTK("\n押码:			%02X%02X%02X%02X",pph->pszMAC[0],pph->pszMAC[1],pph->pszMAC[2],pph->pszMAC[3]);
	PRINTK("\n压缩标志:		%02X",pph->bCompressFlag);
	PRINTK("\n保留:			%02X",pph->bRFU);
	return;
}