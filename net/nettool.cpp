 

#include <stdio.h>


CNetTool::CNetTool()
{
}

CNetTool::~CNetTool()
{
}



int	CNetTool::getComputerName(TCHAR *pszName)
{
	char	temp[100];
	if(gethostname(temp,sizeof(temp))==0)   
	{
		memcpy(pszName,temp,strlen(temp)+1);
		return 0;
	}	
	return -1;
}



int	CNetTool::getIP(TCHAR *pszIP)
{
	BYTE   *p;
	char	temp[100];
	struct  hostent   *hp;

	if(gethostname(temp,sizeof(temp))==0)   
	{   
		if((hp=gethostbyname(temp))!=0)   
		{   
			p=(BYTE   *)hp->h_addr;     
			sprintf(pszIP,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);   

			return 0;
		} 
	}

	return -1;
}


int CNetTool::getMAC(TCHAR * mac)     
{     
	NCB				ncb;
	char			temp[32];

	typedef struct _ASTAT_
	{
		ADAPTER_STATUS	adapt;
		NAME_BUFFER		NameBuff[30];
	}ASTAT,*PASTAT;

	ASTAT	Adapter;
    
	typedef struct _LANA_ENUM
	{   //   le 
		UCHAR   length;
		UCHAR   lana[MAX_LANA];
	}LANA_ENUM;

	LANA_ENUM lana_enum;
	UCHAR		uRetCode;
	
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));
    
	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (unsigned char *)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);
	if(uRetCode != NRC_GOODRET)
		return uRetCode;
    
	for(int lana=0; lana<lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if(uRetCode == NRC_GOODRET)
			break; 
	} 
	if(uRetCode != NRC_GOODRET)
		return uRetCode;     
    
	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = lana_enum.lana[0];
	strcpy((char*)ncb.ncb_callname,"*");
	ncb.ncb_buffer = (unsigned char *)&Adapter;
	ncb.ncb_length = sizeof(Adapter);
	uRetCode = Netbios(&ncb);
	if(uRetCode != NRC_GOODRET)
		return uRetCode;

	memset(temp,0x00,32);
	sprintf(mac,"%02X-%02X-%02X-%02X-%02X-%02X",
		Adapter.adapt.adapter_address[0],
		Adapter.adapt.adapter_address[1],
		Adapter.adapt.adapter_address[2],
		Adapter.adapt.adapter_address[3],
		Adapter.adapt.adapter_address[4],
		Adapter.adapt.adapter_address[5]); 
	return 0;   
} 



