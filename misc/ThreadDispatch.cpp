 


//	CPU��
DWORD	CThreadDispatch::gdwMaxProcessorNum = 1;
//	��ǰ���CPU MASK
DWORD	CThreadDispatch::gdwActiveProcessorMask = 1;
//	��ǰʹ�õ�CPU
DWORD	CThreadDispatch::gdwCurProcessIndex = 0;

CThreadDispatch::CThreadDispatch()
{
	SYSTEM_INFO	sysinfo;
	GetSystemInfo(&sysinfo);
	
	gdwMaxProcessorNum = sysinfo.dwNumberOfProcessors;
	gdwActiveProcessorMask =sysinfo.dwActiveProcessorMask;

	PRINTK("\nCPU��:\t%d",gdwMaxProcessorNum);
	PRINTK("\nMASKS:\t%d",gdwActiveProcessorMask);
}

CThreadDispatch::~CThreadDispatch()
{
}




 
int CThreadDispatch::DispatchThread(HANDLE hThread)
{
	DWORD dwMASK = 0;
	DWORD dwSuccess;

	//	����ǵ��˵ģ����õ���
	if(gdwMaxProcessorNum<2) return 0;
	if(hThread==INVALID_HANDLE_VALUE) return 0;

	while(1)
	{	
		dwMASK = (DWORD)(1<<gdwCurProcessIndex);
		if((dwMASK&gdwActiveProcessorMask)) break;
		
		InterlockedIncrement((LPLONG)&gdwCurProcessIndex);
		if(gdwCurProcessIndex>=gdwMaxProcessorNum) InterlockedExchange((LPLONG)&gdwCurProcessIndex,0);
	}
	dwSuccess = SetThreadAffinityMask(hThread,dwMASK);
	if(dwSuccess)
	{
		PRINTK("\nTHREAD[%d] RUNNING @CPU[%d],PREVIOUS AFFINITY MASK[%08X],NOW AFFINITY MASK[%08X]",
		hThread,gdwCurProcessIndex,dwSuccess,dwMASK);
	}
	else
	{
		PRINTK("\nTHREAD[%d] RUNNING @CPU[%d] FAILURE,AFFINITY MASK[%08X]!",
		hThread,gdwCurProcessIndex,dwMASK);
	}

	InterlockedIncrement((LPLONG)&gdwCurProcessIndex);
	if(gdwCurProcessIndex>=gdwMaxProcessorNum) InterlockedExchange((LPLONG)&gdwCurProcessIndex,0);	

	
	return 0;
}