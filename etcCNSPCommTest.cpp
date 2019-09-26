#define __DEBUG__

#include "etcCNSPComm.cpp"


typedef	struct cmd_func
{
	char	cCmd;
	char	*pszCmdDesc;
	void  (*cmd_proc_func)(void);
}CMD_FUNC;

void		print_cmd_usage(void);

void		test_connect_kms(void);
void		test_disconnect_kms(void);

void		test_signin(void);
void		test_user_card_credit(void);
void		test_user_card_update_file(void);
void		test_obu_update_file(void);
void		test_user_card_validdate(void);
void		test_user_card_debit(void);
void		test_verify_tac(void);
void		test_obu_unload_flag(void);
void		test_user_card_reloadpin(void);

/*	功能列表*/
CMD_FUNC cmd_func_tab[] =
{
	{'Q',"退出",NULL},

	{'L',"测试：连接前置程序",test_connect_kms},
	{'X',"测试：关闭到前置程序的连接",test_disconnect_kms},

	{'N',"测试：操作员签到",test_signin},
	{'C',"测试：用户卡充值",test_user_card_credit},	
	{'D',"测试：用户卡消费",test_user_card_debit},
	{'P',"测试：用户卡二发",test_user_card_update_file},	
	{'J',"测试：用户卡PIN重装",test_user_card_reloadpin},
	{'V',"测试：用户卡修改有效期",test_user_card_validdate},
	{'T',"测试：用户卡验证TAC",test_verify_tac},

	{'2',"测试：OBU二发",test_obu_update_file},
	{'1',"测试：OBU修改拆卸标志",test_obu_unload_flag},	




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


int main(void)
{
	char ch;
	int ret;
	WSADATA		wsaData;  	
	WORD wSockVersion;

	PRINTK("\n高速公路在线密钥服务动态库测试：%s",__DATE__);
	
	//	WinSock2初始化
	wSockVersion = MAKEWORD(2,1);
	ret = WSAStartup(wSockVersion,&wsaData);
	if(ret) 
	{
		CCommServiceLog::LogEvent("\nWIN SOCKET2初始化失败！");
		return ret;
	}

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
	WSACleanup();

	return 0;
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
void		test_disconnect_kms(void)
{
	disconnectOKS();
	return;
}

void		test_signin(void)
{
	int ret;
	char *strOperator="112233445566";

	ret = cmd_1031((BYTE *)strOperator);
	if(ret)
	{
		PRINTK("\n签到失败！");
		return;
	}
	PRINTK("\n签到成功！");

	return;
}

/*
cmd_1032(BYTE bVer,BYTE *szAPPID,
			BYTE *szRnd,WORD wSeqNo,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,DWORD dwRemain,
			BYTE *szMAC1,
			BYTE *szMAC)
*/
void		test_user_card_credit(void)
{

	int ret,i;
	BYTE bVer;
	BYTE szAPPID[8];
	BYTE szRnd[4];
	BYTE szDeviceNo[6],szDateTime[7],szMAC1[4],szMAC2[4];

	bVer = 0x40;

	memcpy(szAPPID,"\x18\x32\x03\x01\x00\x00\x00\x02",8);
	memcpy(szRnd,"\xD0\xF4\x66\xC6",4);
	memcpy(szDeviceNo,"\x64\x01\x00\x00\x01\x02",6);
	memset(szDateTime,0x00,7);
	memcpy(szDateTime,"\x20\x19\x09\x04\x21\x01\x32",7);

	memset(szMAC1,0x00,4);
	memset(szMAC2,0x00,4);
	ret = cmd_1032(bVer,szAPPID,
		szRnd,0x0000,0x000003e8,0x02,szDeviceNo,szDateTime,0,
		szMAC1,
		szMAC2);

	if(ret)
	{
		PRINTK("\n用户卡充值1032 失败:%d",ret);
		return;
	}

	PRINTK("\n用户卡充值1032：%02X%02X%02X%02X",szMAC2[0],szMAC2[1],szMAC2[2],szMAC2[3]);
	PRINTK("\n日期时间：");
	for(i=0;i<7;i++) PRINTK("%02X",szDateTime[i]);
	return;

}

void		test_user_card_update_file(void)
{
	int ret;
	BYTE bVer;
	BYTE szAPPID[8];
	BYTE szRnd[4];
	BYTE szAPDU[128],szMAC[8];

	bVer = 0x40;

	memcpy(szAPPID,"\x18\x32\x03\x01\x00\x00\x00\x02",8);
	memcpy(szRnd,"\xD0\xF4\x66\xC6",4);
	memset(szAPDU,0x00,1);
	memcpy(szAPDU,"\x04\xD6\x96\x00\x3B",5);
	memset(szAPDU+5,0xff,55);

	memset(szMAC,0x00,8);
	ret = cmd_1033(bVer,szAPPID,szRnd,0x01,60,szAPDU,szMAC);
	if(ret)
	{
		PRINTK("\n用户卡二次发行认证1033 失败:%d",ret);
		return;
	}

	PRINTK("\n用户卡二次发行认证1033：%02X%02X%02X%02X",szMAC[0],szMAC[1],szMAC[2],szMAC[3]);
	return;
}

void		test_obu_update_file(void)
{
}

void		test_user_card_validdate(void)
{
	int ret;
	BYTE bVer;
	BYTE szAPPID[8];
	BYTE szRnd[4];
	BYTE szAPDU[32],szMAC[8];

	bVer = 0x40;

	memcpy(szAPPID,"\x18\x32\x03\x01\x00\x00\x00\x02",8);
	memcpy(szRnd,"\xB4\x8C\x41\x70",4);
	memset(szAPDU,0x00,32);
	memcpy(szAPDU,"\x04\xD6\x95\x18\x08\x20\x20\x12\x01",9);

	memset(szMAC,0x00,8);
	ret = cmd_1035(bVer,szAPPID,szRnd,9,szAPDU,szMAC);
	if(ret)
	{
		PRINTK("\n用户卡修改有效期认证 失败:%d",ret);
		return;
	}

	PRINTK("\n用户卡修改有效期认证：%02X%02X%02X%02X",szMAC[0],szMAC[1],szMAC[2],szMAC[3]);
	return;
}

/*
int	__stdcall 	cmd_1036(BYTE bVer,BYTE *szAPPID,
				BYTE *szRnd,
				WORD wSeqNo,DWORD nAuditNo,DWORD nRemain,DWORD nAmount,BYTE bTransFlag,BYTE *szDeviceNo,BYTE *szDateTime,
				BYTE *szMAC)
*/
void		test_user_card_debit(void)
{
	int ret;
	BYTE bVer;
	BYTE szAPPID[8];
	BYTE szRnd[4];
	BYTE szDeviceNo[6],szDateTime[7],szMAC[8];

	bVer = 0x40;

	memcpy(szAPPID,"\x18\x32\x03\x01\x00\x00\x00\x02",8);
	memcpy(szRnd,"\xD0\xF4\x66\xC6",4);
	memcpy(szDeviceNo,"\x64\x01\x00\x00\x01\x02",6);
	memcpy(szDateTime,"\x20\x19\x09\x04\x21\x01\x32",7);

	memset(szMAC,0x00,8);
	ret = cmd_1036(bVer,szAPPID,
		szRnd,
		0x0001,0x00000064,0,10000,0x06,szDeviceNo,szDateTime,
		szMAC);

	if(ret)
	{
		PRINTK("\n用户卡消费1036 失败:%d",ret);
		return;
	}

	PRINTK("\n用户卡消费1036：%02X%02X%02X%02X",szMAC[0],szMAC[1],szMAC[2],szMAC[3]);
	return;
}

void		test_verify_tac(void)
{
}

void		test_obu_unload_flag(void)
{
}

void		test_user_card_reloadpin(void)
{
}
