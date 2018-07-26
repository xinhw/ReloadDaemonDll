


CMisc::CMisc()
{
}

CMisc::~CMisc()
{
}


BOOL CMisc::SystemShutdown(DWORD dwFlag)
{

	HANDLE					hToken; 
	TOKEN_PRIVILEGES		tkp; 
	
	// Get a token for this process. 
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) return( FALSE ); 
	
	// Get the LUID for the shutdown privilege. 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid); 
	
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	
	// Get the shutdown privilege for this process. 
	
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES)NULL, 0); 
	
	if (GetLastError() != ERROR_SUCCESS) return FALSE; 
	
	// Shut down the system and force all applications to close. 
	if (!ExitWindowsEx(dwFlag | EWX_FORCE, 0)) 
		return FALSE; 

	
	return TRUE;
}





BOOL CMisc::SetBoxDatetime(SYSTEMTIME *pstm)
{
	HANDLE					hToken; 
	TOKEN_PRIVILEGES		tkp; 
	
	// Get a token for this process. 
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) return( FALSE ); 
	
	// Get the LUID for the shutdown privilege. 
	LookupPrivilegeValue(NULL, TEXT("SeSystemtimePrivilege"),&tkp.Privileges[0].Luid); 
	
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	
	// Get the shutdown privilege for this process. 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES)NULL, 0); 
	if (GetLastError() != ERROR_SUCCESS) return FALSE; 

	
	return TRUE;
}



bool CMisc::FindFileByName(TCHAR *pszFile)
{
	
	WIN32_FIND_DATA		FindFileData;
	HANDLE				hFind;
	bool				bExisted;
	
	bExisted = true;
	
	hFind = FindFirstFile(pszFile,&FindFileData);
	if(hFind==INVALID_HANDLE_VALUE)
	{
		bExisted = false;
	}
	else
	{
		FindClose(hFind);
	}
	
	return bExisted;
	
}

#define CRC_PRESET 0xffff
#define CRC_POLYNOM 0x8408
WORD CMisc::calcCRC(BYTE *data,UINT length)
{
	WORD crc = CRC_PRESET;
	for(UINT i=0; i<length; i++)
	{
		crc = crc ^ *(data+i);
		for(int j=0;j<8;j++)
		{
			if(crc&0x0001)
				crc=(crc >> 1) ^ CRC_POLYNOM;
			else
				crc = (crc >> 1);
		}
	}
	return crc;
}


BYTE CMisc::bcd2bin(BYTE b)
{
	BYTE ch=0;

	ch = (BYTE)((b>>4)*10 + (b&0x0f));
	return ch;
}

   



BOOL CMisc::GetMemoryInfo(DWORD &dwTotalSpace,DWORD &dwFreeSpace)
{
	MEMORYSTATUS meminfo;

	dwTotalSpace = 0;
	dwFreeSpace = 0;

	memset(&meminfo,0x00,sizeof(MEMORYSTATUS));
	meminfo.dwLength = sizeof(MEMORYSTATUS);
	::GlobalMemoryStatus(&meminfo);


	dwTotalSpace = (DWORD)TOM(meminfo.dwTotalPhys);
	dwFreeSpace = (DWORD)TOM(meminfo.dwAvailPhys);

	return TRUE;
}  


BOOL CMisc::GetDiscInfo(DWORD &dwTotalSpace,DWORD &dwFreeSpace)
{
#ifndef WINCE

	int DiskCount = 0;
	DWORD DiskInfo = GetLogicalDrives();

	dwTotalSpace = 0;
	dwFreeSpace = 0;


	//利用GetLogicalDrives()函数可以获取系统中逻辑驱动器的数量，函数返回的是一个32位无符号整型数据。
	while(DiskInfo)//通过循环操作查看每一位数据是否为1，如果为1则磁盘为真,如果为0则磁盘不存在。
	{
		if(DiskInfo&1)//通过位运算的逻辑与操作，判断是否为1
		{
			++DiskCount;
		}
		DiskInfo = DiskInfo >> 1;//通过位运算的右移操作保证每循环一次所检查的位置向右移动一位。
		//DiskInfo = DiskInfo/2;
	}

	PRINTK(TEXT("\n逻辑磁盘数量:%d"),DiskCount);

	//-------------------------------------------------------------------

	int DSLength = GetLogicalDriveStrings(0,NULL);

	//通过GetLogicalDriveStrings()函数获取所有驱动器字符串信息长度。
	TCHAR* DStr = new TCHAR[DSLength];//用获取的长度在堆区创建一个c风格的字符串数组
	GetLogicalDriveStrings(DSLength,(LPTSTR)DStr);
	//通过GetLogicalDriveStrings将字符串信息复制到堆区数组中,其中保存了所有驱动器的信息。

	int DType;
	int si=0;
	BOOL fResult;
	_int64 i64FreeBytesToCaller;
	_int64 i64TotalBytes;
	_int64 i64FreeBytes;
	for(int i=0;i<DSLength/4;++i)		//为了显示每个驱动器的状态，则通过循环输出实现，由于DStr内部保存的数据是A:NULLB:NULLC:NULL，这样的信息，所以DSLength/4可以获得具体大循环范围
	{
		TCHAR dir[4]={DStr[si],':','\\',0x00};
		PRINTK(TEXT("\n%s"),dir);
		DType = GetDriveType(DStr+i*4);
		//GetDriveType函数，可以获取驱动器类型，参数为驱动器的根目录
		switch(DType)
		{
			case DRIVE_FIXED:
				PRINTK(TEXT("硬盘"));
				break;
			case DRIVE_CDROM:
				PRINTK(TEXT("光驱"));
				continue;
				break;
			case DRIVE_REMOVABLE:
				PRINTK(TEXT("可移动式磁盘"));
				continue;
				break;
			case DRIVE_REMOTE:
				PRINTK(TEXT("网络磁盘"));
				continue;
				break;
			case DRIVE_RAMDISK:
				PRINTK(TEXT("虚拟RAM磁盘"));
				continue;
				break;
			case DRIVE_UNKNOWN:
				PRINTK(TEXT("未知设备"));
				continue;
				break;
			default:
				PRINTK(TEXT("未知设备2"));
				continue;
				break;
		}

		//GetDiskFreeSpaceEx函数，可以获取驱动器磁盘的空间状态,函数返回的是个BOOL类型数据
		fResult = GetDiskFreeSpaceEx (dir,(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);
		if(fResult)//通过返回的BOOL数据判断驱动器是否在工作状态
		{
			PRINTK(TEXT("\nTOTALSPACE:%.2fMB"),(float)i64TotalBytes/1024/1024);			//磁盘总容量
			PRINTK(TEXT("\nFREESPACE:%.2fMB"),(float)i64FreeBytesToCaller/1024/1024);		//磁盘剩余空间

			dwTotalSpace = dwTotalSpace+(DWORD)TOM(i64TotalBytes);
			dwFreeSpace = dwFreeSpace+(DWORD)TOM(i64FreeBytesToCaller);
		}
		else
		{
			PRINTK(TEXT("\n设备未准备好"));
			//return FALSE;
		}
		si+=4;
	}
#else
	dwTotalSpace = dwFreeSpace = 0;

	ULARGE_INTEGER available, total, free;

	//	NandFlash;
	if(GetDiskFreeSpaceEx(TEXT("NandFlash"), &available, &total, &free)==FALSE) return FALSE;
	
	dwTotalSpace = dwTotalSpace + total.u.LowPart;
	dwFreeSpace = dwFreeSpace + free.u.LowPart;




#endif

	

	return TRUE;
}




 
void	CMisc::Int2Bytes(int n,BYTE *pch)
{
	union tagInt2Bytes
	{
		int		k;
		BYTE	ch[4];
	}i2b;

	i2b.k = n;

	pch[0] = i2b.ch[3];
	pch[1] = i2b.ch[2];
	pch[2] = i2b.ch[1];
	pch[3] = i2b.ch[0];

	return;
}

 
void	CMisc::Bytes2Int(BYTE *pch,int *n)
{
	union tagInt2Bytes
	{
		int		k;
		BYTE	ch[4];
	}i2b;
	
	i2b.ch[3] = pch[0];
	i2b.ch[2] = pch[1];
	i2b.ch[1] = pch[2];
	i2b.ch[0] = pch[3];

	*n = i2b.k;
	
	return;
}


UCHAR CMisc::bcd(UCHAR ch)
{
	UCHAR c;

	ch = ch%100;
	c = ch%10;
	c=  c + (ch/10)*16;

	return c;
}



const string CMisc::toHexString(const unsigned char* input, const int datasize)
{
	string output;
	char ch[3];

	for(int i = 0; i < datasize; ++i)
	{
		sprintf(ch, "%02x", input[i]);
		output += ch;
	}
	return output;

} 

void CMisc::xorBytes(int nlen,const BYTE *var1,const BYTE *var2,BYTE *var)
{
	int i;

	for(i=0;i<nlen;i++) var[i] = (BYTE)(var1[i]^var2[i]);
	return;
}


void CMisc::getDateTime(char *s)
{
	SYSTEMTIME st;

	if(NULL==s) return;

	GetLocalTime(&st);

	sprintf(s,"%04d%02d%02d%02d%02d%02d",
			st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	return;
}

void CMisc::getBCDDateTime(BYTE *szDateTime)
{
	char strTime[20];
	int i;

	memset(strTime,0x00,20);
	getDateTime(strTime);

	for(i=0;i<14;i=i+2)
	{
		szDateTime[i/2] = ascToUC(strTime[i])*0x10 + ascToUC(strTime[i+1]);
	}
	return;
}




/*-------------------------------------------------------------------------
Function:		ascToUC
Created:		2018-07-13 17:49:06
Author:			Xin Hongwei(hongwei.xin@avantport.com)
Parameters: 
        
Reversion:
        
-------------------------------------------------------------------------*/
BYTE CMisc::ascToUC(BYTE  ch)
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

void CMisc::StringToByte(char *str,BYTE *arr)
{
	int i,nlen;

	if(arr==NULL||str==NULL) return;

	nlen = strlen(str);

	for(i=0;i<nlen;i=i+2)
	{
		arr[i/2] = ascToUC(str[i])*0x10 + ascToUC(str[i+1]);
	}
	return;
}


void CMisc::ByteToString(BYTE *arr,BYTE nlen,char *str)
{
	BYTE i;

	if(arr==NULL||str==NULL) return;

	for(i=0;i<nlen;i++) sprintf(str+2*i,"%02X",arr[i]);
	return;
}