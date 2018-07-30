
#ifdef CDRDLL_EXPORTS
#define CDRDLL_APICDRDLL_API
#else
#define CDRDLL_API __declspec(dllimport)
#endif

CDRDLL_API   int JT_SamReset ( int iComID, int iSockID, int iBaudrate, int iProtocolType, unsigned char *pReply, int *piLenRep );

CDRDLL_API    int JT_ChangeSamBaudrate ( int iComID, int iSockID, int iBaudrate, int iProtocolType );

CDRDLL_API    int JT_SamCommand( int iComID, int iSockID, const unsigned char *pCommand, int iLenCmd, unsigned char *pReply, int *piLenRep );

CDRDLL_API    int JT_CPUCCommand ( int iComID, const unsigned char *pCommand, int iLenCmd, unsigned char *pReply, int *piLenRep );

CDRDLL_API    int JT_WriteKey ( int iComID, int iKeyType, int iSec, const unsigned char *pKey, const unsigned char *pKeyData );

CDRDLL_API    int JT_WriteFile ( int iComID, int iFileID, int iKeyType, int iStartBlock, int iBlockNum, const unsigned char *pData );

CDRDLL_API    int JT_WriteBlock ( int iComID, int iKeyType, int iBlockn, const unsigned char *pKey, const unsigned char *pData );

CDRDLL_API    int JT_ReadFile ( int iComID, int iFileID, int iKeyType, int iStartBlock, int iBlockNum, unsigned char *pReply );

CDRDLL_API    int JT_ReadBlock ( int iComID, int iKeyType, int iBlockn, const unsigned char *pKey, unsigned char *pReply );

CDRDLL_API   int JT_PurseFileRestore ( int iComID, int iFileId, int iKeyType, int *piMoney );

CDRDLL_API  int JT_PurseRestore ( int iComID, int iSec, int iKeyType, const unsigned char *pKey, int iPurseType, int *piMoney );

CDRDLL_API  int JT_PurseFileCheck( int iComID, int iFileId, int iKeyType, int *piMoney );

CDRDLL_API int JT_PurseCheck(int iComID, int iSec, int iKeyType, const unsigned char *pKey, int iPurseType, int *piMoney );

CDRDLL_API int  JT_PurseFileIncrease(int iComID,int iFileId,int iKeyType,int iMoney1, int *piMoney2 );

CDRDLL_API int  JT_PurseIncrease (int iComID,int iSec,int iKeyType,const unsigned char *pKey,int iMoney1,int *piMoney2 );

CDRDLL_API int JT_PurseFileDecrement( int iComID, int iFileId, int iKeyType, int iMoney1, int *piMoney2 );

CDRDLL_API int JT_PurseDecrement( int iComID, int iSec, int iKeyType, const unsigned char *pKey, int iMoney1, int *piMoney2 );

CDRDLL_API int JT_AudioControl( int iComID, unsigned char cTimes, unsigned char cVoice );

CDRDLL_API int JT_LEDDisplay( int iComID, unsigned char cRed, unsigned char cGreen, unsigned char cBlue );

CDRDLL_API int JT_CloseCard(int iComID);

CDRDLL_API int JT_OpenCard(int iComID, unsigned char * pCardSerialNum, int *piCardSerialNumLength );

CDRDLL_API  int JT_GetLastError(int iComID,char* szLastError,int iMaxLength);

CDRDLL_API void JT_ReaderVersion( int iComID, char* szReaderVersion, int iRVerMaxLength, char* szAPIVersion, int iAPIVerMaxLength );

CDRDLL_API  int JT_CloseReader(int iComID);


CDRDLL_API int JT_OpenReader(int iComID, const char *szParas);
