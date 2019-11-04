#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include "CKeyL2AE.h"

#include <string>
using namespace std;

CKeyL2AE::CKeyL2AE(void)
{
	m_fOpen=FALSE;
}

CKeyL2AE::~CKeyL2AE()
{
	WSACleanup();
}

BOOL CKeyL2AE::Init(void)
{
	if( m_fOpen ) return(TRUE);

  	WSADATA			WSAData;
  	if(WSAStartup( MAKEWORD(2,2), &WSAData)) return(-1);

	m_fOpen=TRUE;

	return(TRUE);
}
//========================================================================================================
//	int CKeyL2AE::L2ARead( char* ipaddr,unsigned short port,char* buf,int bufsize,int devtype,char* devaddr, short counts)
// type: public
// Function: Keyence upper compatible communication
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: Receive buffer
// Argument 4: Receive buffer size
// Argument 5: type of device (0 = word device, 1 = bit device)
// Argument 5: Read start address Device + address No fixed number of digits
// Argument 6: Number of received blocks (Max: 120)
//========================================================================================================
int CKeyL2AE::L2ARead( char* ipaddr,unsigned short port,char* buf,int bufsize,int devtype,char* devaddr, short counts)
{

	char send[5120];
	int ret;


	if( m_fOpen == FALSE ) return(-1);
	if( counts <0 || counts > 120 ) return(-2);	

	if( INADDR_NONE == inet_addr(ipaddr) ) return(-4);

	if(port <1024 || port >65535) return(-5);

	memset( send, 0, sizeof(send) );

	// 1 fixed part
	// Continuous read command "RDE"
	strcpy( send,"RDE ");
	
	// 2 devices + its address
	// DM (data memory), MR (internal relay)
	strcat (send, devaddr);
	
	// 3suffix
	if( devtype==DEV_BIT)
		strcat( send,".U");	// bit device
	strcat( send, " " );	

	// 3 read number
	sprintf( &send[strlen(send)],"%d",counts);

	// end cr
	strcat( send, "\r" );	

	ret=UdpSend(ipaddr,port,send,strlen(send),buf,bufsize);
	
	char buf2[500]={0};
	char buf3[20]={0};
	for(int i=0 ; i< counts ; i++){
		memcpy(buf3, buf+(6*i),5 );
		buf3[5]=0;
		sprintf( buf2+strlen(buf2), "%04X", atoi(buf3) );
	}

	strcpy(buf,buf2);

	return(ret);
}

//========================================================================================================
//	int CKeyL2AE::P3EWrite(char* ipaddr,unsigned short port,int devtype,char* devaddr, short counts, char* data)
// type: public
// Function: Block transmission of device data by upper compatible communication
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: type of device (0 = word device, 1 = bit device)
// Argument 4: Writing start address (8 digits)
// Argument 5: Number of transmit blocks (Max: 120) Receive buffer
// Argument 6: write data (for ASCII code, 2 characters for ASCII code (4 digits in 2 ~ 2), BCD for numeric value)
//========================================================================================================
int CKeyL2AE::L2AWrite(char* ipaddr,unsigned short port,int devtype,char* devaddr, short counts, char* data)
{

	char send[5120];
	int ret;
	char buf[40+1];

	if( m_fOpen == FALSE ) return(-1);

	if( counts <0 || counts > 120 ) return(-2);

	if( INADDR_NONE == inet_addr(ipaddr) ) return(-4);

	if(port <1024 || port >65535) return(-5);

	if( devtype==DEV_WORD){
		if( (short)strlen(data) !=counts*4) return(-6);
	}
	else{
		if( (short)strlen(data) !=counts) return(-7);
	}
	memset( send, 0, sizeof(send) );

	// 1 fixed part
	// Continuous writing command "WDE"
	strcpy( send,"WRE ");
	
	// 2 devices + its address
	// DM (data memory), MR (internal relay)
	strcat( send, devaddr );
	
	// suffix
	if( devtype==DEV_BIT)
		strcat( send,".U");	// bit device
	strcat( send, " " );	

	// Number of readouts
	sprintf( &send[strlen(send)],"%d",counts);
	
	// 4 write data
	for(int i=0; i< counts;i++){
		strcat( send, " " );	

		memset( buf,0,sizeof(buf) );
		memcpy( buf, data+(i*4), 4 );

		int data =0 ;
				
		data = htoi( buf , 4 );
		sprintf(buf,"%d",data);
		memcpy( send+strlen(send), buf, strlen(buf) );	
	}

	// end cr
	strcat( send, "\r" );	

	ret=UdpSend(ipaddr,port,send,strlen(send),buf,sizeof(buf));

	return(ret);
}


int CKeyL2AE::L2ASetBit(char* ipaddr,unsigned short port,char* devaddr,BOOL on )
{

	char send[128];
	int ret;
	char buf[64];

	if( m_fOpen == FALSE ) return(-1);

	if( INADDR_NONE == inet_addr(ipaddr) ) return(-4);

	if(port <1024 || port >65535) return(-5);

	memset( send, 0, sizeof(send) );

	// 1 fixed part
	// SET or RESET
	if( on ) strcpy( send,"ST ");
	else strcpy( send,"RS ");

	strcat( send, devaddr );

	strcat( send, "\r" );	

	ret=UdpSend(ipaddr,port,send,strlen(send),buf,sizeof(buf));
	if(ret) return(-1);
	if( memcmp( buf,"OK",2)) return(1);

	return(ret);
}

int  CKeyL2AE::GetPlcType(char* ipaddr,unsigned short port,char* recvBuf)
{
	string stbuf;
	int ret=0;

	stbuf = "?K\r";

	ret=UdpSend(ipaddr,port,(char*)stbuf.c_str() ,stbuf.length() ,recvBuf,3);

	return ret;
}

int CKeyL2AE::UdpSend( char* ipaddr,unsigned short port,char* data,int size,char* rcv,int rsize)
{
	SOCKET fSocket;
	WSAEVENT fEvent;

	unsigned long laddr;
	WSABUF		wsBuf;			
	DWORD		dwBufferCount;	
	DWORD		cnt;			
	DWORD		dwFlags;		
	struct sockaddr_in	addr;
	int ret;
	WSANETWORKEVENTS events;
	int			iFromlen;		
	char rbuf[4096];


	if( INADDR_NONE ==( laddr=inet_addr(ipaddr)) ) return(-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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


	wsBuf.len=size;
	wsBuf.buf=data;
	dwBufferCount=1;
	dwFlags=MSG_DONTROUTE;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= laddr;
	addr.sin_port			= htons(port);


	ret=WSASendTo(fSocket,&wsBuf,dwBufferCount, &cnt,dwFlags,
					(LPSOCKADDR)&addr,sizeof(addr),NULL,NULL);
	if (ret == SOCKET_ERROR){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}


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


	if((events.lNetworkEvents & FD_READ)==0){
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}


	memset( rbuf,0,sizeof(rbuf));
	wsBuf.len=sizeof(rbuf);
	wsBuf.buf=rbuf;
	dwBufferCount=1;
	dwFlags=0;
	memset(&addr, 0, sizeof(addr));
	iFromlen=sizeof(addr);

	ret=WSARecvFrom(fSocket,&wsBuf,dwBufferCount, &cnt,&dwFlags,
							(LPSOCKADDR)&addr, &iFromlen,NULL,NULL);

	WSACloseEvent(fEvent);
	closesocket(fSocket);

	if( strlen(rbuf) > 0 ){
		if( rsize  > strlen(rbuf)+1 )
			strcpy(rcv,rbuf);
		else
			memcpy(rcv,rbuf,rsize);
	}

	return(0);
}

int CKeyL2AE::Str2Mel(char* buf, unsigned int bufsize, char* str)
{
	unsigned int i,len;
	char txt[10];

	len = strlen(str);
	if( len*2 >= bufsize ) return(-1);

	memset( buf, 0, bufsize );

	for( i=0;i<len;i++){
		if( i+1==len )
			wsprintf( txt,"00");
		else
			wsprintf( txt,"%02X",(unsigned char)str[i+1]);

		memcpy( &buf[i*2], txt, 2 );
		wsprintf( txt,"%02X",(unsigned char)str[i]);		
		i++;
		memcpy( &buf[i*2], txt, 2 );
	}

	return(0);

}
int CKeyL2AE::Mel2Str(char* buf, unsigned int bufsize, char* melstr)
{
	unsigned int i,len;

	len = strlen(melstr);
	if( bufsize<=len/2 ) return(-1);

	memset( buf, 0, bufsize );

	for( i=0;i<len;i+=2){
		if( i+2>=len )
			buf[strlen(buf)]=0x20;
		else
			buf[strlen(buf)]=(char)htoi( &melstr[i+2],2);

		buf[strlen(buf)]=(char)htoi( &melstr[i],2);
		i+=2;
	}
	return(0);
}

unsigned short CKeyL2AE::htoi( char *hexstr , short len )
{
	unsigned short ret = 0;
	short i;

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


BOOL CKeyL2AE::ULChg( char* buf )
{
	char tmp[8];

	if( strlen(buf)<8 )
		return FALSE;

	memcpy( tmp , buf , 8);
	memcpy( buf+0 , tmp+4 , 4 );
	memcpy( buf+4 , tmp+0 , 4 );
	
	return TRUE;
}
