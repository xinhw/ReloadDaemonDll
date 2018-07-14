
#pragma once

class CThreadDispatch
{
public:
	CThreadDispatch();
	~CThreadDispatch();
private:
	//	CPU数
	static DWORD	gdwMaxProcessorNum;
	//	当前活动的CPU MASK
	static DWORD	gdwActiveProcessorMask;
	//	当前使用的CPU
	static DWORD	gdwCurProcessIndex;

public:
	static int DispatchThread(HANDLE hThread);
};