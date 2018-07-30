/*-------------------------------------------------------------------------
    Shanghai AvantPort Information Technology Co., Ltd

    Software Development Division

    Xin Hongwei(hongwei.xin@avantport.com)

    Created：2018/07/19 10:32:20

    Reversion:
			通用的读卡器基类，以确保后续可能要使用有其他读卡器
-------------------------------------------------------------------------*/
#pragma once


class CCardReader
{
private:
	double m_dTimeSum;
public:
	virtual UINT Open(char *strAddress, unsigned int iBaud) = 0;
	virtual UINT Initialize(BYTE *strsno,BYTE &bATSLen,BYTE *strResult) = 0;
	virtual void Close() = 0;
	virtual UINT RunCmd(char *strCmd, char *strResult) = 0;
	virtual UINT Halt()=0;
	
	void WriteReaderLog(char * strText)
	{
		CCommServiceLog::LogEvent(strText);
		return;
	};

	void SetTimeSum(double dTimeSum)
	{
		m_dTimeSum = dTimeSum;
	};

	double GetTimeSum()
	{
		return m_dTimeSum;
	};

	void AddTimeSum(double dTimeSum)
	{
		m_dTimeSum += dTimeSum;
	};
};
