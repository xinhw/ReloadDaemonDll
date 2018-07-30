#pragma once

#ifndef UBYTE
#define UBYTE BYTE
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//	3DES加解密函数
///////////////////////////////////////////////////////////////////////////////////////////////////	
void __stdcall des_encode(UBYTE *key,UBYTE *sr,UBYTE *dest);
void __stdcall des_decode(UBYTE *key,UBYTE *sr,UBYTE *dest);
void __stdcall des3_encode(UBYTE *key,UBYTE *sr,UBYTE *dest);
void __stdcall des3_decode(UBYTE *key,UBYTE *sr,UBYTE *dest);

/*完整性保护*/
void __stdcall CmdWatchCalMac(UINT	nLenIn,UCHAR	*pszBufIn,
						UCHAR	*pszInitData,
						UCHAR	*pszKey, 
						UCHAR	*pszMAC,
						bool  bTriDes);

/*机密性保护加密函数*/
void __stdcall CmdWatchEncode(UCHAR	nLenIn,UCHAR	*pszBufIn,
						UCHAR	*pszKey,
						UCHAR	*pszBufOut,
						bool 	bTriDes);

/*机密性保护解密函数*/
void __stdcall CmdWatchDecode(UCHAR	nLenIn,UCHAR	*pszBufIn,
						UCHAR	*pszKey,
						UCHAR	*pszBufOut,
						bool	bTriDes);

/*分散算法*/
void __stdcall WatchDiversity(UCHAR	*pszMKKey,UCHAR		*pszPID,UCHAR	*pszSKKey,BOOL bTriDes);


///////////////////////////////////////////////////////////////////////////////////////////////////
//	SM4加解密函数
///////////////////////////////////////////////////////////////////////////////////////////////////	
void __stdcall sm4_encode(BYTE *szKey, BYTE *szSrc, BYTE *szDest);
void __stdcall sm4_decode(BYTE *szKey, BYTE *szSrc, BYTE *szDest);

void __stdcall sm4_cal_mac(BYTE *szKey,BYTE *szInit,int nLen,BYTE *szData,BYTE *szMAC);

void __stdcall sm4_data_encode(UCHAR	nLenIn,UCHAR	*pszBufIn,
							UCHAR	*pszKey,
							UCHAR	*pszBufOut);

void __stdcall sm4_data_decode(UCHAR	nLenIn,UCHAR	*pszBufIn,
							UCHAR	*pszKey,
							UCHAR	*pszBufOut);

void __stdcall sm4_diversify(BYTE *szKey,BYTE *szDID,BYTE *szSUBKey);
