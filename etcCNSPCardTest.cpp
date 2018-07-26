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

/*	功能列表*/
CMD_FUNC cmd_func_tab[] =
{
	{'Q',"退出",NULL},
	{'O',"测试：打开读卡器",test_open_reader},
	{'L',"测试：连接前置程序",test_connect_kms},

	{'X',"测试：关闭读卡器",test_close_reader},

	{'S',"测试：用户卡复位",test_search_card},

	{'R',"测试：用户卡读信息",test_cpucard_read},
	{'I',"测试：用户卡一发",test_cpucard_init},
	{'P',"测试：用户卡二发",test_cpucard_personize},
	{'C',"测试：用户卡充值",test_cpucard_credit},
	{'D',"测试：用户卡消费",test_cpucard_debit},

	{'B',"测试：OBU复位",test_obu_reset},
	{'0',"测试：OBU读卡信息",test_obu_read},
	{'1',"测试：OBU一发",test_obu_init},
	{'2',"测试：OBU二发",test_obu_personize},

	{'N',"测试：操作员签到",test_signin},

	{'U',"用法",print_cmd_usage},
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

	PRINTK("\n高速公路在线密钥服务动态库测试：%s",__DATE__);

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

	PRINTK("\n运行结束,Press Any Key to continue...\n");
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
	
	PRINTK("\n\n\n用法如下：");
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

	PRINTK("\n请输入读卡器类型(0--航天金卡读写器，1--万集OBU读写器\n");
	scanf("%d",&ntype);

	PRINTK("\n输入串口号：");
	scanf("%d",&ncom);
	PRINTK("\n输入波特率：");
	scanf("%d",&nbaud);

	ret = openReader(ntype,ncom,nbaud);
	if(ret)
	{
		PRINTK("\n读卡器打开失败：%d",ret);
	}
	else
	{
		PRINTK("\n读卡器打开成功！");
	}

	return;
}

void		test_connect_kms(void)
{
	int ret;

	char strip[32];
	int nport;

	memset(strip,0x00,32);
	PRINTK("\n请输入前置IP:");
	scanf("%s",strip);

	PRINTK("\n请输入前置端口:");
	scanf("%d",&nport);

	ret = connectOKS(strip,nport);
	if(ret)
	{
		PRINTK("\n连接前置服务程序失败:%d",ret);
		return;
	}

	PRINTK("\n连接前置服务程序成功！");
	return;

}



void		test_close_reader(void)
{
	int ret;

	ret = closeReader();
	if(ret)
	{
		PRINTK("\n读卡器关闭失败：%d",ret);
	}
	else
	{
		PRINTK("\n读卡器关闭成功！");
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
		PRINTK("\n没找到卡片");
		return;
	}

	PRINTK("\n卡唯一号：%02X%02X%02X%02X",szSNO[0],szSNO[1],szSNO[2],szSNO[3]);
	PRINTK("\n复位信息：");
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
		PRINTK("\n读用户卡失败:%d",ret);
		return;
	}

	PRINTK("\n0015文件：\n");
	for(i=0;i<50;i++) PRINTK("%02X ",elf15[i]);

	PRINTK("\n0016文件：\n");
	for(i=0;i<55;i++) PRINTK("%02X ",elf16[i]);

	PRINTK("\n余额：%d",dwRemain);

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
		PRINTK("\n一发失败：%4X",ret);
		return;
	}

	PRINTK("\n一发成功！");

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
		PRINTK("\n读用户卡失败:%d",ret);
		return;
	}

	bVer= elf15[9];
	memcpy(szAPPID,elf15+12,8);

	ret = cpuCredit(bVer,szAPPID,10000,szTransTime,wSeqNo,szTAC);
	if(ret)
	{
		PRINTK("\n圈存失败：%4X",ret);
		return;
	}

	PRINTK("\n圈存成功！");

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
		PRINTK("\n没找到OBU");
		return;
	}

	PRINTK("\nOBU复位信息：");
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
		PRINTK("\n读取OBU信息失败！");
		return;
	}

	PRINTK("\n读取0BU信息成功：");
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
		PRINTK("\n签到失败！");
		return;
	}
	PRINTK("\n签到成功！");

	return;
}