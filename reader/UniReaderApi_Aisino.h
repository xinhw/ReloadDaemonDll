
#pragma once


int __stdcall READER_open(char *Paras);
int __stdcall READER_close(long DevHandle);
int __stdcall SAM_reset(long DevHandle,int iSockID,int* iReplylength,char* sReply);
int __stdcall SAM_command(long DevHandle,int iSockID,int iCommandLength,char* sCommand,int* iReplylength,char* sReply);
int __stdcall CARD_open(long DevHandle,int RequestMode,char* PhysicsCardno,char* ResetInformation,int* CardPlace,char* CardType);
int __stdcall CARD_close(long DevHandle);
int __stdcall PRO_command(long DevHandle,int CardPlace,int iCommandLength,char* sCommand,int* iReplylength, char* sReply);
int __stdcall ICC_authenticate(long DevHandle, int CardPlace,int sector,int keytype,char* key);
int __stdcall ICC_readsector(long DevHandle,int CardPlace,int sector,int start,int len,char* data);
int __stdcall ICC_writesector(long DevHandle,int CardPlace,int sector,int start,int len,char* data);
int __stdcall GetReaderVersion (long DevHandle,unsigned char *sReaderVersion,int iRVerMaxLength, unsigned char *sAPIVersion,int iAPIVerMaxlength);
int __stdcall Led_display(long DevHandle,unsigned char cRed,unsigned char cGreen, unsigned char cBlue);
int __stdcall Audio_control(long DevHandle,unsigned char cBeep);
char* __stdcall GetOpInfo(int retcode);

//////////////////////////////////////////////////////////////////
bool __stdcall GetCardNo_RFID(char* CardNo);
bool __stdcall GetCPCID_RFID(char* CPCID);
bool __stdcall GetFlagStationInfo_RFID(char* CPCID, char* InitData, int* FlagStationCnt, char* FlagStationInfo); 
bool __stdcall GetPowerInfo_RFID(char* CPCID, int* iPowerInfor);
bool __stdcall Set433CardMode_RFID(char* CPCID, int iMode);
bool __stdcall Get433CardMode_RFID(char* CPCID, int* iMode);
////////////////////////////////////////////////////////////////////////////////////
int __stdcall  _WriteReaderID(char *ReaderID);
int __stdcall  _GetReaderID(char *ReaderID);
////////////////////////////////////////////////////////////////////////////////////
//extern "C" __declspec(dllimport) 
