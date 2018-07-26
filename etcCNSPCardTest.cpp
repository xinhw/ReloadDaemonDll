#define __DEBUG__

#define DEBUG_TEST
#define	DEBUG_ICMD
#define DEBUG_PRINT

#include "dllinclude.hpp"

#include "etcCNSPCard.cpp"

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

void		test_obu_reset(void);
void		test_obu_read(void);
void		test_obu_init(void);
void		test_obu_personize(void);

void		test_signin(void);

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

	{'B',"���ԣ�OBU��λ",test_obu_reset},
	{'0',"���ԣ�OBU������Ϣ",test_obu_read},
	{'1',"���ԣ�OBUһ��",test_obu_init},
	{'2',"���ԣ�OBU����",test_obu_personize},

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



void		test_open_reader(void)
{
	int ret,ntype;
	int ncom,nbaud;

	PRINTK("\n���������������(0--����𿨶�д����1--��OBU��д��\n");
	scanf("%d",&ntype);

	PRINTK("\n���봮�ںţ�");
	scanf("%d",&ncom);
	PRINTK("\n���벨���ʣ�");
	scanf("%d",&nbaud);

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

	ret = connectOKS(strip,nport);
	if(ret)
	{
		PRINTK("\n����ǰ�÷������ʧ��:%d",ret);
		return;
	}

	PRINTK("\n����ǰ�÷������ɹ���");
	return;

}



void		test_close_reader(void)
{
	int ret;

	ret = closeReader();
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


void		test_search_card(void)
{
	int ret;
	BYTE bLen,i;
	BYTE szSNO[10],szATS[64];

	bLen = 0;
	memset(szATS,0x00,64);

	ret = cpuATS(szSNO,bLen,szATS);
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


void		test_cpucard_read(void)
{
	int ret,i;
	BYTE elf15[50],elf16[55];
	DWORD dwRemain;

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain);
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


void		test_cpucard_init(void)
{
	int ret,i,n;
	BYTE elf15[50];

	//	C4FECFC4C4FECFC4 01 50 6401 6666660018474415 2018010120181231C4FE4144353337320000000001030000000000000000
	char *p15 = "C4FECFC4C4FECFC40150640166666600184744152018010120181231C4FE4144353337320000000001030000000000000000";

	memset(elf15,0x00,50);

	n = strlen(p15);
	for(i=0;i<n;i=i+2)
	{
		elf15[i/2] = CMisc::ascToUC(p15[i])*0x10 + CMisc::ascToUC(p15[i+1]);
	}

	ret = cpuInit(elf15);
	if(ret)
	{
		PRINTK("\nһ��ʧ�ܣ�%4X",ret);
		return;
	}

	PRINTK("\nһ���ɹ���");

	return;
}


void		test_cpucard_personize(void)
{
	return;
}


void		test_cpucard_credit(void)
{
	int ret;
	BYTE elf15[50],elf16[55];
	BYTE szTransTime[7],szTAC[4];
	WORD wSeqNo;
	DWORD dwRemain;
	BYTE bVer,szAPPID[8];

	memset(elf15,0x00,50);
	memset(elf16,0x00,55);
	dwRemain = 0;

	ret =cpuReadCardFiles(elf15,elf16,dwRemain);
	if(ret)
	{
		PRINTK("\n���û���ʧ��:%d",ret);
		return;
	}

	bVer= elf15[9];
	memcpy(szAPPID,elf15+12,8);

	ret = cpuCredit(bVer,szAPPID,10000,szTransTime,wSeqNo,szTAC);
	if(ret)
	{
		PRINTK("\nȦ��ʧ�ܣ�%4X",ret);
		return;
	}

	PRINTK("\nȦ��ɹ���");

	return;
}

void		test_cpucard_debit(void)
{
	return;
}


void		test_obu_reset(void)
{
	int ret;
	BYTE bLen,i;
	BYTE szATS[64];

	ret = obuATR(bLen,szATS);
	if(ret)
	{
		PRINTK("\nû�ҵ�OBU");
		return;
	}

	PRINTK("\nOBU��λ��Ϣ��");
	for(i=0;i<bLen;i++) PRINTK("%02X",szATS[i]);

	return;
}

void		test_obu_read(void)
{
	int ret,i;
	BYTE elf01_mk[100],elf01_adf01[80];

	memset(elf01_mk,0x00,100);
	memset(elf01_adf01,0x00,80);

	ret = obuRead(elf01_mk,elf01_adf01);
	if(ret)
	{
		PRINTK("\n��ȡOBU��Ϣʧ�ܣ�");
		return;
	}

	PRINTK("\n��ȡ0BU��Ϣ�ɹ���");
	PRINTK("\nELF01[MK]:");
	for(i=0;i<30;i++) PRINTK("%02X ",elf01_mk[i]);


	return;
}


void		test_obu_init(void)
{
	return;
}

void		test_obu_personize(void)
{
	return;
}


void		test_signin(void)
{
	int ret;
	char *strOperator="112233445566";

	ret = signIn(strOperator);
	if(ret)
	{
		PRINTK("\nǩ��ʧ�ܣ�");
		return;
	}
	PRINTK("\nǩ���ɹ���");

	return;
}