
#ifndef		_ERRORCODE_H_
#define		_ERRORCODE_H_

	//          SOCKET创建和连接错误 
	#define     ERR_SOCKET_CREATE			0x200
	#define     ERR_SOCKET_CONNECT			0x201
	#define		ERR_SOCKET_CONN_OVERTIME	0x202     

	//          发送和接受错误
	#define     ERR_SOCKET_INVALID			0x300
	#define     ERR_NO_SEND_SOCKET			0x301
	#define     ERR_SEND_FAILURE			0x302
	#define     ERR_NOT_SEND_OVER			0x303
	#define     ERR_NO_RECV_SOCKET			0x304
	#define     ERR_RECV_NOTHING			0x305
	#define     ERR_RECV_FAILURE			0x306

	#define		ERR_TAC_CODE				0x400
	#define		ERR_PARAM					0x500
	#define		ERR_DATAGRAM				0x501
	#define		ERR_UNSUPPORT				0x502

	#define ERR_APP_LENGTH					0x0401		//	报文长度错误
	#define	ERR_ENCODER						0x00ec		//	加密机错误
	#define	ERR_DATABASE					0x00db		//	数据库错误

	#define ERR_M1_KEY_NUM					0x0402;
	#define ERR_M1_CARD_MAC					0x0403

#endif
