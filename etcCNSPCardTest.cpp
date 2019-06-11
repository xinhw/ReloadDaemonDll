/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created��2018/07/27 10:17:43

    Reversion:
        
-------------------------------------------------------------------------*/
#define __DEBUG__

#define DEBUG_TEST
#define	DEBUG_ICMD
#define DEBUG_PRINT
//#define DEBUG_TCP_PRINT

/*
//	����Ҫ�ѣ�WS2_32.lib EncodeDLL.lib etcCNSPCard.lib ���뵽������
#include "etcCNSPCard.hpp"
*/

#ifndef PRINTK
#define PRINTK printf
#endif


#include "dllinclude.hpp"
#include "etcCNSPCard.cpp"

int gnCom=0;

typedef	struct cmd_func
{
	char	cCmd;
	char	*pszCmdDesc;
	void  (*cmd_proc_func)(void);
}CMD_FUNC;

void		print_cmd_usage(void);
void		test_open_reader(void);
void		test_connect_kms(void);
void		test_close_reader(void);
void		test_search_card(void);
void		test_cpucard_read(void);
void		test_cpucard_init(void);
void		test_cpucard_personize(void);
void		test_cpucard_credit(void);
void		test_cpucard_debit(void);
void		test_cpucard_update000e(void);
void		test_cpucard_reload_pin(void);
void		test_cpucard_readadfile(void);


void		test_obu_read(void);
void		test_obu_init(void);
void		test_obu_pre_init(void);
void		test_obu_personize(void);
void		test_obu_unlock_adf(void);

void		test_signin(void);

void		test_obu_decode_plate(void);
void		test_psam_auth(void);

/*	�����б�*/
CMD_FUNC cmd_func_tab[] =
{
	{'Q',"�˳�",NULL},
	{'O',"���ԣ��򿪶�����",test_open_reader},
	{'L',"���ԣ�����ǰ�ó���",test_connect_kms},

	{'X',"���ԣ��رն�����",test_close_reader},

	{'S',"���ԣ��û�����λ",test_search_card},

	{'R',"���ԣ��û�������Ϣ",test_cpucard_read},
	{'I',"���ԣ��û���һ��",test_cpucard_init},
	{'P',"���ԣ��û�������",test_cpucard_personize},
	{'C',"���ԣ��û�����ֵ",test_cpucard_credit},
	{'D',"���ԣ��û�������",test_cpucard_debit},
	{'A',"���ԣ��û�������000E�ļ�",test_cpucard_update000e},
	{'J',"���ԣ��û���PIN��װ",test_cpucard_reload_pin},
	{'E',"���ԣ��û�����ȡADF�ļ�",test_cpucard_readadfile},

	{'Y',"���ԣ�OBU��Ԥ����",test_obu_pre_init},

	{'0',"���ԣ�OBU������Ϣ",test_obu_read},
	{'1',"���ԣ�OBUһ��",test_obu_init},
	{'2',"���ԣ�OBU����",test_obu_personize},
	{'3',"���ԣ�OBU����Ӧ��",test_obu_unlock_adf},
	{'4',"���ԣ�OBU������Ϣ����",test_obu_decode_plate},

	{'5',"���ԣ�PSAM����Ȩ",test_psam_auth},

	{'N',"���ԣ�����Աǩ��",test_signin},

	{'U',"�÷�",print_cmd_usage},
	{0,0,0}
};

/*-------------------------------------------------------------------------
Function:		cmd_exec
Created:		2018-07-11 09:59:30
Author:			Xin Hongwei(xinhw@me.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		cmd_exec(char cmd)
{
	CMD_FUNC		*pc;
	
	for (pc = cmd_func_tab; pc->cCmd; pc++)
	{
		if(cmd==pc->cCmd)
		{
			PRINTK("\n-----------------------------------------------------------------");
			PRINTK("\n%s",pc->pszCmdDesc);
			PRINTK("\n-----------------------------------------------------------------");

			pc->cmd_proc_func();	
			break;
		}
	}
	
	return;
}






/*-------------------------------------------------------------------------
Function:		main
Created:		2018-07-11 09:59:35
Author:			Xin Hongwei(xinhw@me.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
int main(void)
{
	char ch;

	PRINTK("\n���ٹ�·������Կ����̬����ԣ�%s",__DATE__);

	print_cmd_usage();

	while(1)
	{
		ch = getch();
		if(ch=='Q'||ch=='q') break;

		ch = toupper(ch);

		cmd_exec(ch);

		PRINTK("\n\n\n");
	}

	disconnectOKS();

	PRINTK("\n���н���,Press Any Key to continue...\n");
	getch();

	return 0;
}



/*-------------------------------------------------------------------------
Function:		print_cmd_usage
Created:		2018-07-11 09:59:39
Author:			Xin Hongwei(xinhw@me.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		print_cmd_usage(void)
{
	CMD_FUNC		*pc;
	
	PRINTK("\n\n\n�÷����£�");
	for (pc = cmd_func_tab; pc->cCmd; pc++)
	{
		PRINTK("\n%C=%s",pc->cCmd,pc->pszCmdDesc);
	}
	PRINTK("\n\n\n");
	
	return;
}



/*-------------------------------------------------------------------------
Function:		test_open_reader
Created:		2018-07-27 10:17:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_open_reader(void)
{
	int ret,ntype;
	int ncom,nbaud;

	PRINTK("\n���������������\n\t0--����𿨶�д��\n\t1--��OBU��д��\n\t2--�����۵۶�����\n\t3--���ڽ���OBU��д��\n");
	scanf("%d",&ntype);

	PRINTK("\n���봮�ںţ�");
	scanf("%d",&ncom);
	PRINTK("\n���벨���ʣ�");
	scanf("%d",&nbaud);

	gnCom = ncom;

	ret = openReader(ntype,ncom,nbaud);
	if(ret)
	{
		PRINTK("\n��������ʧ�ܣ�%d",ret);
	}
	else
	{
		PRINTK("\n�������򿪳ɹ���");
	}

	return;
}

/*-------------------------------------------------------------------------
Function:		test_connect_kms
Created:		2018-07-27 10:17:58
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_connect_kms(void)
{
	int ret;

	char strip[32];
	int nport;

	memset(strip,0x00,32);
	PRINTK("\n������ǰ��IP:");
	scanf("%s",strip);

	PRINTK("\n������ǰ�ö˿�:");
	scanf("%d",&nport);

	ret = connectOKS(strip,nport,gnCom);
	if(ret)
	{
		PRINTK("\n����ǰ�÷������ʧ��:%d",ret);
		return;
	}

	PRINTK("\n����ǰ�÷������ɹ���");
	return;

}



/*-------------------------------------------------------------------------
Function:		test_close_reader
Created:		2018-07-27 10:18:02
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_close_reader(void)
{
	int ret;

	ret = closeReader(gnCom);
	if(ret)
	{
		PRINTK("\n�������ر�ʧ�ܣ�%d",ret);
	}
	else
	{
		PRINTK("\n�������رճɹ���");
	}

	return;
}


/*-------------------------------------------------------------------------
Function:		test_search_card
Created:		2018-07-27 10:18:05
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_search_card(void)
{
	int ret;
	BYTE bLen,i;
	BYTE szSNO[10],szATS[64];

	bLen = 0;
	memset(szATS,0x00,64);
	memset(szSNO,0x00,4);

	ret = cpuATS(szSNO,bLen,szATS,gnCom);
	if(ret)
	{
		PRINTK("\nû�ҵ���Ƭ");
		return;
	}

	PRINTK("\n��Ψһ�ţ�%02X%02X%02X%02X",szSNO[0],szSNO[1],szSNO[2],szSNO[3]);
	PRINTK("\n��λ��Ϣ��");
	for(i=0;i<bLen;i++) PRINTK("%02X",szATS[i]);

	return;
}


/*-------------------------------------------------------------------------
Function:		test_cpucard_read
Created:		2018-07-27 10:18:08
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_cpucard_read(void)
{
	int ret,i;
	BYTE elf15[50],elf16[55];
	DWORD dwRemain;

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain,gnCom);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	PRINTK("\n0015�ļ���\n");
	for(i=0;i<50;i++) PRINTK("%02X ",elf15[i]);

	PRINTK("\n0016�ļ���\n");
	for(i=0;i<55;i++) PRINTK("%02X ",elf16[i]);

	PRINTK("\n��%d",dwRemain);

	return;
}

/*-------------------------------------------------------------------------
Function:		ascToUC
Created:		2018-07-27 10:18:08
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
BYTE ascToUC(BYTE  ch)
{
	BYTE value;
	
	switch(ch){
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		value = ch - '0';
		break;
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		value = ch - 'a'+10;
		break;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		value = ch - 'A'+10;
		break;
		
	default:
		value = 0;
		break;
    }
	return value;
}


/*-------------------------------------------------------------------------
Function:		test_cpucard_init
Created:		2018-07-27 10:18:11
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_cpucard_init(void)
{
	int ret,i,n;
	BYTE elf15[50];

	//	C4FECFC4C4FECFC4 01 50 6401 6666660018474415 2018010120181231C4FE4144353337320000000001030000000000000000
	char *p15 = "C4FECFC4C4FECFC40150640166666612184744152018010120181231C4FE4144353337320000000001030000000000000000";

	memset(elf15,0x00,50);

	n = strlen(p15);
	for(i=0;i<n;i=i+2)
	{
		elf15[i/2] = ascToUC(p15[i])*0x10 + ascToUC(p15[i+1]);
	}

	ret = cpuInit(elf15,gnCom);
	if(ret)
	{
		PRINTK("\nһ��ʧ�ܣ�%4X",ret);
		return;
	}

	PRINTK("\nһ���ɹ���");

	return;
}


/*-------------------------------------------------------------------------
Function:		test_cpucard_personize
Created:		2018-07-27 10:18:15
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_cpucard_personize(void)
{
	int ret;
	BYTE elf15[50],elf16[55];
	DWORD dwRemain;
	BYTE bVer,szAPPID[8];

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain,gnCom);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	//	����0015�ļ�
	ret = cpuUpdateIssueFile(bVer,szAPPID,elf15,gnCom);
	if(ret)
	{
		PRINTK("\n���� 0015 �ļ�ʧ��:%d",ret);
		return;
	}

	//	����0016�ļ�
	ret = cpuUpdateUserFile(bVer,szAPPID,elf16,gnCom);
	if(ret)
	{
		PRINTK("\n���� 0016 �ļ�ʧ��:%d",ret);
		return;
	}

	//	������Ч����
	ret = cpuUpdateValidDate(bVer,szAPPID,(BYTE *)"\x20\x20\x12\x01",gnCom);
	if(ret)
	{
		PRINTK("\n�����û�����Ч����:%d",ret);
		return;
	}
	

	return;
}


/*-------------------------------------------------------------------------
Function:		test_cpucard_credit
Created:		2018-07-27 10:18:18
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_cpucard_credit(void)
{
	int ret;
	BYTE elf15[50],elf16[55];
	BYTE szTransTime[7],szTAC[4];
	WORD wSeqNo;
	DWORD dwRemain;
	BYTE bVer,szAPPID[8];
	BYTE szDeviceNo[6];

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain,gnCom);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	bVer= elf15[9];
	memcpy(szAPPID,elf15+12,8);

	memset(szDeviceNo,0x00,6);

	ret = cpuCredit(bVer,szAPPID,10000,szTransTime,szDeviceNo,wSeqNo,szTAC,gnCom);
	if(ret)
	{
		PRINTK("\nȦ��ʧ�ܣ�%4X",ret);
		return;
	}

	PRINTK("\nȦ��ɹ���");

	return;
}

/*-------------------------------------------------------------------------
Function:		test_cpucard_debit
Created:		2018-07-27 10:18:22
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
       int __stdcall cpuPurchase(BYTE bVer,BYTE *szAPPID,DWORD dwAmount,DWORD dwAuditNo,BYTE *szDateTime,BYTE *szDeviceNo,
						WORD &wSeqNo,BYTE *szTAC,int ncom) 
-------------------------------------------------------------------------*/
void		test_cpucard_debit(void)
{
	int ret;
	BYTE elf15[50],elf16[55];
	BYTE szTransTime[7],szTAC[4];
	WORD wSeqNo;
	DWORD dwRemain;
	BYTE bVer,szAPPID[8];
	BYTE szDeviceNo[6];

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain,gnCom);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	bVer= elf15[9];
	memcpy(szAPPID,elf15+12,8);

	memset(szDeviceNo,0x00,6);

	CMisc::getBCDDateTime(szTransTime);
	
	ret = cpuPurchase(bVer,szAPPID,100,1838,szTransTime,szDeviceNo,wSeqNo,szTAC,gnCom);
	if(ret)
	{
		PRINTK("\n����ʧ�ܣ�%4X",ret);
		return;
	}

	PRINTK("\n���ѳɹ���");	

	return;
}

/*-------------------------------------------------------------------------
Function:		test_obu_read
Created:		2018-07-27 10:18:28
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_obu_read(void)
{
	int ret,i;
	BYTE elf01_mk[100],elf01_adf01[256];
	BYTE szAPPID[8];
	BYTE szPlainText[128],bVer;

	memset(elf01_mk,0x00,100);
	memset(elf01_adf01,0x00,80);

	ret = obuGetUID(szAPPID,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBUUIDʧ��:%d",ret);
		return;
	}

	PRINTK("\nOBU UID:%02X%02X%02X%02X",szAPPID[0],szAPPID[1],szAPPID[2],szAPPID[3]);

	ret = obuRead(elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBU ϵͳ��Ϣ�ļ� ʧ�ܣ�%d",ret);
		return;
	}

	bVer = elf01_mk[9];

	memset(szPlainText,0x00,128);
	ret = obuReadVehicleFile(1,bVer,szPlainText,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBU ������Ϣ�ļ� ʧ�ܣ�%d",ret);
		return;
	}

	PRINTK("\n��ȡ0BU��Ϣ�ɹ���");

	PRINTK("\nϵͳ��Ϣ�ļ�:");
	for(i=0;i<30;i++) PRINTK("%02X ",elf01_mk[i]);

	PRINTK("\n������Ϣ�ļ�:");
	for(i=0;i<59;i++) PRINTK("%02X ",szPlainText[i]);

	return;
}


/*-------------------------------------------------------------------------
Function:		test_obu_init
Created:		2018-07-27 10:18:31
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_obu_init(void)
{
	int ret;
	BYTE elf01_mk[100],elf01_adf01[256];

	memset(elf01_mk,0x00,100);
	memset(elf01_adf01,0x00,256);

	//	�ȶ�����һ��OBU����Ϣ
	ret = obuRead(elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBU��Ϣʧ�ܣ�");
		return;
	}

	memset(elf01_adf01,0x00,80);
	memcpy(elf01_adf01,"��A-12345",9);
	elf01_adf01[14]=0x01;
	elf01_adf01[15]=0x00;

	//	Ȼ���ʼ��
	ret= obuInit(elf01_mk,elf01_adf01,gnCom);
	if(ret)
	{
		PRINTK("\nOBU��ʼ��ʧ�ܣ�");
		return;
	}
	PRINTK("\nOBU��ʼ���ɹ���");

	return;
}

/*-------------------------------------------------------------------------
Function:		test_obu_personize
Created:		2018-07-27 10:18:34
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_obu_personize(void)
{
	int ret;
	BYTE elf01_mk[100],elf01_adf01[256];
	BYTE bVer,szAPPID[8];

	memset(elf01_mk,0x00,100);
	memset(elf01_adf01,0x00,80);

	ret = obuRead(elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBU��Ϣʧ�ܣ�");
		return;
	}

	bVer = elf01_mk[9];
	memcpy(szAPPID,elf01_mk+10,8);

	PRINTK("\n��ʼ���� ϵͳ��Ϣ�ļ�");
	ret = obuUpdateFile(bVer,szAPPID,1,elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\n���� ϵͳ��Ϣ�ļ� ʧ��:%04X",ret);
		return;
	}
	/*	*/
	memset(elf01_adf01,0x00,80);
	memcpy(elf01_adf01,"��A-12345",9);
	elf01_adf01[14]=0x01;
	elf01_adf01[15]=0x00;

	PRINTK("\n��ʼ���� ������Ϣ�ļ�");
	ret = obuUpdateFile(bVer,szAPPID,2,elf01_adf01,gnCom);
	if(ret)
	{
		PRINTK("\n���� ������Ϣ�ļ� ʧ��:%04X",ret);
		return;
	}

	
	PRINTK("\n��ʼ���� ��ж��־");
	ret = obuUpdateLoadFlag(bVer,szAPPID,0x01,gnCom);
	if(ret)
	{
		PRINTK("\n���� ��ж��־ ʧ��:%04X",ret);
		return;
	}
	return;
}



/*-------------------------------------------------------------------------
Function:		test_obu_unlock_adf
Created:		2018-08-04 13:33:07
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        		OBU����ADF
-------------------------------------------------------------------------*/
void		test_obu_unlock_adf(void)
{

	int ret;
	BYTE elf01_mk[100],elf01_adf01[256];
	BYTE bVer,szAPPID[8];

	memset(elf01_mk,0x00,100);
	memset(elf01_adf01,0x00,80);

	ret = obuRead(elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBU��Ϣʧ�ܣ�");
		return;
	}

	bVer = elf01_mk[9];
	memcpy(szAPPID,elf01_mk+10,8);

	ret = obuUnlockApplication(bVer,szAPPID,gnCom);
	if(ret)
	{
		PRINTK("\nӦ�ý���ʧ�ܣ�");
		return;
	}
	PRINTK("\nӦ�ý����ɹ���");
}

/*-------------------------------------------------------------------------
Function:		test_obu_pre_init
Created:		2018-08-04 13:32:49
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        		OBUԤ��������
-------------------------------------------------------------------------*/
void		test_obu_pre_init(void)
{
	int ret;
	BYTE elf01_mk[100],elf01_adf01[256];
	WORD wDFID = 0x0000;

	memset(elf01_mk,0x00,100);
	memset(elf01_adf01,0x00,256);

	//	�ȶ�����һ��OBU����Ϣ
	ret = obuRead(elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡOBU��Ϣʧ�ܣ�");
		return;
	}

	//	Ȼ���ʼ��
	PRINTK("\nԤ����ϵͳ������Կ...");
	wDFID = 0x3f00;
	ret= obuPreInit(wDFID,elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\nOBU��%04XԤ��ʼ��ʧ�ܣ�",wDFID);
		return;
	}
	PRINTK("\nϵͳ������ԿԤ����ɹ���");

	wDFID = 0xDF01;
	PRINTK("\nԤ����Ӧ��������Կ...");
	ret= obuPreInit(wDFID,elf01_mk,gnCom);
	if(ret)
	{
		PRINTK("\nOBU��%04XԤ��ʼ��ʧ�ܣ�",wDFID);
		return;
	}

	PRINTK("\nӦ��������ԿԤ����ɹ���");
	PRINTK("\nOBUԤ������ɣ�����ִ��һ��");

	return;
}




/*-------------------------------------------------------------------------
Function:		test_signin
Created:		2018-07-27 10:18:37
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_signin(void)
{
	int ret;
	char *strOperator="112233445566";

	ret = signIn(strOperator,gnCom);
	if(ret)
	{
		PRINTK("\nǩ��ʧ�ܣ�");
		return;
	}
	PRINTK("\nǩ���ɹ���");

	return;
}





/*-------------------------------------------------------------------------
Function:		test_cpucard_update000e
Created:		2018-07-27 10:18:37
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_cpucard_update000e(void)
{
	int ret;
	BYTE elf15[50],elf16[55];
	DWORD dwRemain;
	BYTE bVer,szAPPID[8];
	BYTE szFile000E[70];

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain,gnCom);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	memset(szFile000E,0x00,70);

	ret = cpuUpdateFile000E(bVer,szAPPID,szFile000E,gnCom);
	if(ret)
	{
		PRINTK("\nCPU�����η��в����ļ���֤ʧ�ܣ�");
		return;
	}
	PRINTK("\nCPU�����η��в����ļ���֤�ɹ���");
	return;
}


/*-------------------------------------------------------------------------
Function:		test_cpucard_reload_pin
Created:		2018-07-28 11:00:55
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_cpucard_reload_pin(void)
{
	int ret;
	BYTE elf15[50],elf16[55];
	DWORD dwRemain;
	BYTE bVer,szAPPID[8];

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain,gnCom);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	bVer = elf15[9];
	memcpy(szAPPID,elf15+12,8);

	ret = cpuReloadPIN(bVer,szAPPID,6,(BYTE *)"123456",gnCom);
	if(ret)
	{
		PRINTK("\nPIN����ʧ�ܣ�");
	}
	else
	{
		PRINTK("\nPIN�����ɹ���");
	}


	return;
}


/*-------------------------------------------------------------------------
Function:		test_cpucard_readadfile
Created:		2018-08-27 11:34:10
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void test_cpucard_readadfile(void)
{
	int ret,i;
	BYTE elf[256];


	PRINTK("\n��ȡ000E�ļ���");
	memset(elf,0x00,256);
	ret = cpuReadAdfFile(0x0e,0,0x46,elf,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡ 000E �ļ�ʧ��:%04X",ret);
		return;
	}
	for(i=0;i<0x46;i++)
	{
		if(i%32==0) PRINTK("\n");
		PRINTK("%02X ",elf[i]);
	}

	PRINTK("\n��ȡ0008�ļ���");
	memset(elf,0x00,256);
	ret = cpuReadAdfFile(0x08,0,128,elf,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡ 0008 �ļ�ʧ��:%04X",ret);
		return;
	}
	for(i=0;i<128;i++)
	{
		if(i%32==0) PRINTK("\n");
		PRINTK("%02X ",elf[i]);
	}


	PRINTK("\n��ȡ0009�ļ���");
	memset(elf,0x00,256);
	ret = cpuReadAdfFile(0x09,0,128,elf,gnCom);
	if(ret)
	{
		PRINTK("\n��ȡ 0009 �ļ�ʧ��:%04X",ret);
		return;
	}
	for(i=0;i<128;i++)
	{
		if(i%32==0) PRINTK("\n");
		PRINTK("%02X ",elf[i]);
	}
	

	return;
}








/*-------------------------------------------------------------------------
Function:		test_obu_decode_plate
Created:		2018-08-27 11:34:05
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_obu_decode_plate(void)
{
	int ret;
	BYTE szAPPID[8],bLen,i;
	BYTE szData[128];

	//	������Ϣ�ļ�������
	BYTE *szEncData=(BYTE*)"\xD2\xC8\x46\x8E\x9B\xDE\xBA\xA4\xBF\xBC\x18\xB5\x10\x83\x91\x1D" \
					 "\x9E\x4E\xED\x41\x4D\x54\xD7\xA9\x4F\x63\x74\xD3\x4F\x1C\xA1\x7F" \
					 "\xB1\x3B\xE7\xEE\xA1\xC0\x67\xF3\xA5\x88\x7A\x05\xFA\x45\xF1\xDA" \
					 "\xB1\x3B\xE7\xEE\xA1\xC0\x67\xF3\xA5\x88\x7A\x05\xFA\x45\xF1\xDA" \
					 "\x02\x6E\x43\xCA\x8A\xA9\x89\x1D\x6C\x67\x26\xC3\xD7\xEF\xB6\x2A";
	
	memset(szAPPID,0x00,8);
	
	bLen = 0x00;
	memset(szData,0x00,128);

	ret = obuOnlineDecodePlate(0x51,szAPPID,0x43,0x50,szEncData,&bLen,szData,gnCom);
	if(ret)
	{
		PRINTK("\nOBU���߽��ܳ�����Ϣ�ļ�ʧ�ܣ�%d",ret);
		return;
	}

	PRINTK("\n����[%d]��",bLen);
	for(i=0;i<bLen;i++) PRINTK("%02X",szData[i]);
	return;
}




/*-------------------------------------------------------------------------
Function:		test_psam_auth
Created:		2018-08-27 11:34:02
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
void		test_psam_auth(void)
{
	int ret;
	BYTE szSAMNo[10],szRnd[8],i;
	BYTE bAPDULen,szAPDU[128];
	char strListNo[40];

	setTimeout(10,gnCom);

	memcpy(szSAMNo,"\x64\x01\x01\x01\x00\x00\x00\x00\x00\x01",10);
	memcpy(szRnd,"\x25\xFC\xBC\x35\x66\x8B\xBD\xD2",8);

	//	0. PSAM����ǩ��
	ret = psamOnlineSignIn(szSAMNo,szSAMNo+4,0x183,"��������",0x63,"���շ�վ",0x01,0x02,0x10,(BYTE *)"\x20\x18\x11\x14\x16\x08\x12",gnCom);
	if(ret)
	{
		PRINTK("\nPSAM����ǩ��ʧ�ܣ�%d",ret);
		return;
	}

	bAPDULen = 0x00;
	memset(szAPDU,0x00,128);

	//	1. PSAM������Ȩ����
	memset(strListNo,0x00,40);
	ret = psamOnlineAuth(szSAMNo,szRnd,0x183,"��������",0x63,"���շ�վ",0x01,0x02,0x10,&bAPDULen,szAPDU,strListNo,gnCom);
	if(ret)
	{
		PRINTK("\nPSAM������֤ʧ�ܣ�%d",ret);
		return;
	}

	PRINTK("\nLISTNO:%s",strListNo);

	PRINTK("\n����[%d]��",bAPDULen);
	for(i=0;i<bAPDULen;i++) PRINTK("%02X ",szAPDU[i]);

	//	2. PSAM������Ȩȷ��
	ret = psamOnlineAuthConfirm(szSAMNo,strListNo,0x9000,0x00,gnCom);
	if(ret)
	{
		PRINTK("\nPSAM������Ȩȷ��ʧ�ܣ�%d",ret);
		return;
	}

	return;
}
