
#pragma once

class CThreadDispatch
{
public:
	CThreadDispatch();
	~CThreadDispatch();
private:
	//	CPU��
	static DWORD	gdwMaxProcessorNum;
	//	��ǰ���CPU MASK
	static DWORD	gdwActiveProcessorMask;
	//	��ǰʹ�õ�CPU
	static DWORD	gdwCurProcessIndex;

public:
	static int DispatchThread(HANDLE hThread);
};