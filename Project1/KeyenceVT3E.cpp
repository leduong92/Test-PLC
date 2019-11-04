#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include "KeyenceVT3e.h"


KeyenceVT3e::KeyenceVT3e(void)
{
	m_fOpen=FALSE;
}

KeyenceVT3e::~KeyenceVT3e()
{
	WSACleanup();
}


BOOL KeyenceVT3e::Init(void)
{

	if( m_fOpen ) return(TRUE);

  	WSADATA			WSAData;


  	if(WSAStartup( MAKEWORD(2,2), &WSAData)) return(-1);

	m_fOpen=TRUE;

	return(TRUE);
}

int KeyenceVT3e::ReadW(const char* ipaddr ,const int port
				,const int  dev_no,const int cnt,char* buf,unsigned int bufsize)
{
	char send[256],rcv[256];
	int ret;
	unsigned short* rcvdat;

	if( cnt <1 || cnt > 64 ) return(-1);
	if( (int)bufsize < cnt*4) return(-2);


	// Prepare the header
	VT_UDP_SEND* vt=(VT_UDP_SEND*)send;
	memset(send,0,sizeof(send));
	memcpy( vt->cmd,"ERW",3);		// command
	vt->siki='M';					// Identifier M: Internal device fixed
	vt->dev_no=dev_no;				// Device address
	vt->dev_cnt=cnt;				// The number of data
	vt->len=7;						// Transmission data length (read only, so header only)

	// To receive
	ret=UdpSend((char*)ipaddr,port,send,sizeof(VT_UDP_SEND),rcv,sizeof(rcv));
	if(ret) return(ret);

	//Result check
	VT_UDP_RCV* vtr=(VT_UDP_RCV*)rcv;
	if( vtr->result ) return((int)vtr->result);

	// Set the position of received data
	rcvdat=(unsigned short*)(rcv+sizeof(VT_UDP_RCV));

	// Copy data
	memset( buf,0,bufsize);
	for(ret=0;ret<cnt;ret++,rcvdat++){
		sprintf( &buf[ret*4],"%04X",*rcvdat);
	}

	return(0);
}
//========================================================================================================
// int KeyenceVT3e :: WriteW (char * dev, int cnt, char * buf, unsigned int bufsize)
// Function: write out TP internal word device
// Argument 1: IP address
// Argument 2: port number
// Argument 1: Device address
// Argument 2: number of devices (1-64)
// Argument 3: Transmission data (1 word = 4 byte string format same as Melsec)
//========================================================================================================
int KeyenceVT3e::WriteW(const char* ipaddr ,const int port,const int  dev_no,const int cnt,char* dat)
{
	char send[256],rcv[256];
	int ret;
	unsigned short* senddat;

	// Argument check
	if( cnt <1 || cnt > 64 ) return(-1);
	if( (int)strlen(dat) < cnt*4) return(-2);


	// Prepare the header
	VT_UDP_SEND* vt=(VT_UDP_SEND*)send;
	memset(send,0,sizeof(send));
	memcpy( vt->cmd,"EWW",3);		// command
	vt->siki='M';					// Identifier M: Internal device fixed
	vt->dev_no=dev_no;				// Device address
	vt->dev_cnt=cnt;				// The number of data
	vt->len=7+cnt*2;				// Transmission data length (header + data)

	// Data set: convert string to number
	senddat=(unsigned short*)(send+sizeof(VT_UDP_SEND));
	for(ret=0;ret<cnt;ret++,senddat++){
		*senddat=this->htous(&dat[ret*4],4);
	}

	// Send
	ret=UdpSend((char*)ipaddr,port,send,sizeof(VT_UDP_SEND)+cnt*2,rcv,sizeof(rcv));
	if(ret) return(ret);

	// Result check
	VT_UDP_RCV* vtr=(VT_UDP_RCV*)rcv;
	if( vtr->result ) return((int)vtr->result);


	return(0);
}


//========================================================================================================
// int KeyenceVT3e::ReadB(char* dev,int cnt,char* buf,unsigned int bufsize)
// Function: Read internal word device of TP
// Argument 1: IP address
// Argument 2: port number
// Argument 3: Device address
// Return value 0 = OFF, 1 = ON, other = error
//========================================================================================================
int KeyenceVT3e::ReadB(const char* ipaddr ,const int port,const int  dev_no)
{
	char send[256],rcv[256];
	int ret;
	unsigned char* rcvdat;

	VT_UDP_SEND_BIT* vt=(VT_UDP_SEND_BIT*)send;
	memset(send,0,sizeof(send));
	memcpy( vt->cmd,"ERB",3);		
	vt->siki='M';					
	vt->dev_no=dev_no;				
	vt->len=6;						

	ret=UdpSend((char*)ipaddr,port,send,sizeof(VT_UDP_SEND_BIT),rcv,sizeof(rcv));
	if(ret) return(ret);

	VT_UDP_RCV* vtr=(VT_UDP_RCV*)rcv;
	if( vtr->result ) return(-1);

	rcvdat=(unsigned char*)(rcv+sizeof(VT_UDP_RCV));

	ret=(int)(*rcvdat);

	return(ret);
}

//========================================================================================================
// int KeyenceVT3e :: ReadB (char * dev, int cnt, char * buf, unsigned int bufsize)
// Function: Read internal word device of TP
// Argument 1: IP address
// Argument 2: port number
// Argument 3: Device address
// Argument 4: TRUE = on, FALSE = OFF
// return value 0 = OK, otherwise = error
//========================================================================================================
int KeyenceVT3e::WriteB(const char* ipaddr ,const int port,const int  dev_no,const BOOL on)
{
	char send[256],rcv[256];
	int ret;
	unsigned char* senddat;


	//Prepare the header
	VT_UDP_SEND_BIT* vt=(VT_UDP_SEND_BIT*)send;
	memset(send,0,sizeof(send));
	memcpy( vt->cmd,"EWB",3);		
	vt->siki='M';					
	vt->dev_no=dev_no;				
	vt->len=6+1;					


	senddat=(unsigned char*)(send+sizeof(VT_UDP_SEND_BIT));
	if( on ) *senddat=1;


	ret=UdpSend((char*)ipaddr,port,send,sizeof(VT_UDP_SEND_BIT)+1,rcv,sizeof(rcv));
	if(ret) return(ret);

	
	VT_UDP_RCV* vtr=(VT_UDP_RCV*)rcv;
	if( vtr->result ) return(vtr->result);

	return(ret);
}

//========================================================================================================
//	int KeyenceVT3e::UdpSend( char* ipaddr,unsigned short port,char* data,int size,char* rcv,int rsize)
// type: private
// Function: UDP transmission / reception
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: Transmission data
// Argument 4: Transmission data size
// Argument 5: Receive buffer
// Argument 6: Receive buffer size
// Return value: 0 = OK, less than 0 = network error, 1 = received data error
//========================================================================================================
int KeyenceVT3e::UdpSend( char* ipaddr,unsigned short port,char* data,int size,char* rcv,unsigned int rsize)
{
	SOCKET fSocket;
	WSAEVENT fEvent;
	//DWORD		cEvents;		
	unsigned long laddr;
	WSABUF		wsBuf;		
	DWORD		dwBufferCount;	//lpwsBuf Number of WSABUF structures in the array indicated by
	DWORD		cnt;//A pointer to a DWORD variable that receives the number of bytes to send
	DWORD		dwFlags;		//Bit mask to control transmission method
	struct sockaddr_in	addr;
	int ret;
	WSANETWORKEVENTS events;
	int			iFromlen;//An integer specifying the length of the sockaddr structure indicated by lpTo
	char rbuf[4096];

	// IP(to 32bit)
	if( INADDR_NONE ==( laddr=inet_addr(ipaddr)) ) return(-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	fSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (fSocket == INVALID_SOCKET){
		printf("socket()Err"); 
		return(-102);
	}
	fEvent = WSACreateEvent();
	if(fEvent == WSA_INVALID_EVENT){
		printf("\nWSACreateEvent()Err: %d\n", fEvent);
		closesocket(fSocket);
		return(-103);
	}

	ret = WSAEventSelect(fSocket,fEvent,FD_READ);
	if(ret == SOCKET_ERROR){
		printf("\nWSAEventSelect()Err: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-104);
	}

	// Transmission data creation
	wsBuf.len=size;
	wsBuf.buf=data;
	dwBufferCount=1;
	dwFlags=MSG_DONTROUTE;//MSG_PARTIAL;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= laddr;
	addr.sin_port			= htons(port);

	//Send
	ret=WSASendTo(fSocket,&wsBuf,dwBufferCount, &cnt,dwFlags,
					(LPSOCKADDR)&addr,sizeof(addr),NULL,NULL);
	if (ret == SOCKET_ERROR){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}

	// Waiting for reception
	//ret = WSAWaitForMultipleEvents(1,&fEvent,FALSE,10000,FALSE);
	//ret = WSAWaitForMultipleEvents(2,&fEvent,TRUE,10000,TRUE);
	ret = WSAWaitForMultipleEvents(1,&fEvent,TRUE,10000,TRUE);	
	if(ret == WSA_WAIT_TIMEOUT){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-106);
	}

	ret = WSAEnumNetworkEvents(fSocket,fEvent,&events);
	if(ret == SOCKET_ERROR){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-107);
	}

	// Other than received event
	if((events.lNetworkEvents & FD_READ)==0){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}

	// To receive
	//printf("Event Err FD=%d\n",FD_READ);
	memset( rbuf,0,sizeof(rbuf));
	wsBuf.len=sizeof(rbuf);
	wsBuf.buf=rbuf;
	dwBufferCount=1;
	dwFlags=0;//MSG_PEEK;//MSG_PARTIAL;
	memset(&addr, 0, sizeof(addr));
	iFromlen=sizeof(addr);

	ret=WSARecvFrom(fSocket,&wsBuf,dwBufferCount, &cnt,&dwFlags,
							(LPSOCKADDR)&addr, &iFromlen,NULL,NULL);
	WSACloseEvent(fEvent);
	closesocket(fSocket);

	if(ret == SOCKET_ERROR){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}

	memset( rcv,0,rsize);
	memcpy( rcv, rbuf, cnt> rsize ? rsize:cnt);

	return(0);
}

//========================================================================================================
// unsigned short KeyenceVT3e :: htous (char * hexstr, short len)
// Function: Convert hexadecimal (only 2 character string) to integer value (unsigned char type)
// Argument: Conversion source data storage pointer
// Return value: Converted data
//========================================================================================================
unsigned short KeyenceVT3e::htous( char *hexstr , short len )
{
	unsigned short ret = 0;
	unsigned short i;

	if( len > 4 ) return 0;
	for( i = 0 ; i < len ; i++ ){
		ret *= 16;
		if( hexstr[i] < 'A' ) ret += hexstr[i] - '0';
		else
		if( hexstr[i] < 'a' ) ret += hexstr[i] - 'A'+10;
		else
			ret += hexstr[i] - 'a'+10;
	}
	return(ret);
}

