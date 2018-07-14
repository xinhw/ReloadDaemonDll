
 
//#pragma once
//class __declspec(dllimport) CTCPServer;
//#pragma comment(lib, "./Debug/transfer.lib")
 
class _declspec(dllimport)  CTCPServer 

{
	public:
		CTCPServer();
		CTCPServer(char *strIP,WORD wPort);
		CTCPServer(SOCKADDR_IN *pServer);
		CTCPServer(SOCKET nsock);
		~CTCPServer();

	private:
		UINT			gMaxWaitTime;
		SOCKADDR_IN		m_saddr;


	public:
		SOCKET			m_socket;

		//		初始化连接
		int		HD_Init();	
		//		终止连接
		int		HD_Term();

		int     send_recv(WORD nInLen,BYTE *pszBufIn, 
						WORD *nOutLen,BYTE *pszBufOut,
						SOCKADDR_IN	*pRemoteAddr);

		int		s_send(UINT	rBytes,UCHAR	*pszBuf);
		int		s_recv(WORD  nBuffSize ,UINT	*rBytes,UCHAR	*pszBuf,UINT	nWaitMaxTime);

		int		socket_waittm(UINT		nWaitMaxTime,BYTE nMode);


		int		get_recv_buff(WORD nBuffSize,WORD *nRetLen,BYTE *pszBuf,UINT nWaitTime);

	private:

		void	flush_recv_buff(void);

};
 
