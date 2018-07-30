#pragma once

#ifndef UBYTE
#define UBYTE BYTE
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//	3DES�ӽ��ܺ���
///////////////////////////////////////////////////////////////////////////////////////////////////	
void __stdcall des_encode(UBYTE *key,UBYTE *sr,UBYTE *dest);
void __stdcall des_decode(UBYTE *key,UBYTE *sr,UBYTE *dest);
void __stdcall des3_encode(UBYTE *key,UBYTE *sr,UBYTE *dest);
void __stdcall des3_decode(UBYTE *key,UBYTE *sr,UBYTE *dest);

/*�����Ա���*/
void __stdcall CmdWatchCalMac(UINT	nLenIn,UCHAR	*pszBufIn,
						UCHAR	*pszInitData,
						UCHAR	*pszKey, 
						UCHAR	*pszMAC,
						bool  bTriDes);

/*�����Ա������ܺ���*/
void __stdcall CmdWatchEncode(UCHAR	nLenIn,UCHAR	*pszBufIn,
						UCHAR	*pszKey,
						UCHAR	*pszBufOut,
						bool 	bTriDes);

/*�����Ա������ܺ���*/
void __stdcall CmdWatchDecode(UCHAR	nLenIn,UCHAR	*pszBufIn,
						UCHAR	*pszKey,
						UCHAR	*pszBufOut,
						bool	bTriDes);

/*��ɢ�㷨*/
void __stdcall WatchDiversity(UCHAR	*pszMKKey,UCHAR		*pszPID,UCHAR	*pszSKKey,BOOL bTriDes);


///////////////////////////////////////////////////////////////////////////////////////////////////
//	SM4�ӽ��ܺ���
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
