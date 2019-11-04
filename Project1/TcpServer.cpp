#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "TcpServer.h"
#include "stdio.h"
#include "getnow.h"

TcpServer::TcpServer(int port)
{
	m_Status=0;
	m_PortNo = port;
	ITcpServer();
}

TcpServer::~TcpServer()
{
	if(m_Status==1){
		closesocket( m_FdAcpt );
		closesocket( m_Fd );
		WSACleanup();
	}
}

TcpServer::TcpServer()
{
	m_Status=0;
	m_PortNo = -1;

}

int TcpServer::Initialize(int portNo,int PortCheck)
{
	if( PortCheck==TRUE ){
		if( portNo <1024) m_PortNo = DEF_TRANSPORT_NO;
		else m_PortNo = portNo;
	}else{
		m_PortNo = portNo;
	}
  	if ((m_Status = WSAStartup( MAKEWORD(2,2), &WSAData )) == 0){
  	}
  	else{
    	printf( "\nWSAStartup(%d) error", m_Status );
    	return -1;
  	}

  	if ((m_FdAcpt = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    	printf( "\nsocket() error" );
    	return -2;
  	}


  	memset(&m_AddrAcpt, 0, sizeof(m_AddrAcpt));  /* bzero(&addr, sizeof(addr)); */
  	m_AddrAcpt.sin_family      = PF_INET;
  	m_AddrAcpt.sin_port        = htons(m_PortNo);
  	m_AddrAcpt.sin_addr.s_addr = INADDR_ANY;
  	if (bind(m_FdAcpt, (struct sockaddr FAR *) &m_AddrAcpt, sizeof(m_AddrAcpt)) == SOCKET_ERROR) {
    	printf( "\nbind() error" );
    	closesocket( m_FdAcpt );
    	return -3;
  	}
	//4. Create event.
	m_Event = WSACreateEvent();
	if(m_Event == WSA_INVALID_EVENT)
	{
		closesocket(m_FdAcpt);
        WSACleanup();
		return -4;
	}


	int err;
	err = WSAEventSelect(m_FdAcpt,m_Event,FD_ACCEPT );
	if(err == SOCKET_ERROR)
	{
        printf("\nWSAEventSelect()Err: %d\n", err);
		WSACloseEvent(m_Event);
		closesocket(m_FdAcpt);
		WSACleanup();
		return -5;
	}

	err = listen(m_FdAcpt, 1); 
  	if ( err == SOCKET_ERROR) {
		printf( "\nlisten() error=%d",WSAGetLastError() );
    	closesocket(m_FdAcpt);
    	return -6;
  	}	
	m_Status=1;
	return 0;
}


BOOL  TcpServer::ITcpServer()
{
	memset(m_ClientIp,0,sizeof(m_ClientIp));

  	printf( "\n" );
  	printf( "<<<   TCP Protocol Setup   >>>\n" );
  	printf( "\n" );

  	if ((m_Status = WSAStartup( MAKEWORD(2,2), &WSAData )) == 0){
		printf( "%s\n", WSAData.szSystemStatus);
  	}
  	else{
    	printf( "WSAStartup(%d) error", m_Status );
    	return FALSE;
  	}


  	if ((m_FdAcpt = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    	printf( "socket() error\n" );
    	return FALSE;
  	}

  	memset(&m_AddrAcpt, 0, sizeof(m_AddrAcpt));  /* bzero(&addr, sizeof(addr)); */
  	m_AddrAcpt.sin_family      = PF_INET;
  	m_AddrAcpt.sin_port        = htons(m_PortNo);
  	m_AddrAcpt.sin_addr.s_addr = INADDR_ANY;
  	if (bind(m_FdAcpt, (struct sockaddr FAR *) &m_AddrAcpt, sizeof(m_AddrAcpt)) == SOCKET_ERROR) {
    	printf( "bind() error\n" );
    	closesocket( m_FdAcpt );
    	return FALSE;
  	}
	//4. Create event.
	m_Event = WSACreateEvent();
	if(m_Event == WSA_INVALID_EVENT)
	{
		closesocket(m_FdAcpt);
        WSACleanup();
		return FALSE;
	}

	int err;
	err = WSAEventSelect(m_FdAcpt,m_Event,FD_ACCEPT );
	if(err == SOCKET_ERROR)
	{
        printf("\nWSAEventSelect()Err: %d\n", err);
		WSACloseEvent(m_Event);
		closesocket(m_FdAcpt);
		WSACleanup();
		return FALSE;
	}

	err = listen(m_FdAcpt, 1);  
	if ( err == SOCKET_ERROR) {
		printf( "listen() error=%d\n",WSAGetLastError() );
    	closesocket(m_FdAcpt);
    	return -1;
  	}	
	m_Status=1;
	return TRUE;
}

void TcpServer::GetClientIp(char *ip)
{
	strcpy(ip,m_ClientIp);
}

int TcpServer::Wait( DWORD timeout)
{
	WSAEVENT	hEvents[5];		
	DWORD		cEvents;		
	WSANETWORKEVENTS events;
    int ret=0;
	

	hEvents[0] = m_Event;
	cEvents=1;
	events.lNetworkEvents=0;
	ret = WSAWaitForMultipleEvents(cEvents,&m_Event,FALSE,timeout,FALSE);

    if (ret == WSA_WAIT_TIMEOUT)
    {
         printf(".");	
         return(-1);
    }
    ret = WSAEnumNetworkEvents(m_FdAcpt,m_Event,&events);
   if (ret == SOCKET_ERROR)
   {
        printf("WSAEnumNetworkEvents():Err=%d\n",WSAGetLastError());getchar();
        return(-2);
   }
#ifdef _DEBUGPRINT
printf("ret=%d event=%d\n",ret,events.lNetworkEvents);
#endif
    if((events.lNetworkEvents & FD_ACCEPT)==0)
	{
		printf("Event Err FD=%d\n",FD_ACCEPT);return(-3);
	}
    return(ret);
}
int  TcpServer::Recv( char *recvdat )
{
  	int		exit_flag;

	int		recv_len;
	int		err;
  	int len = sizeof(m_AddrAcpt);
	char buf[5+1];
	char revbuf[MAX_SENDLENGTH];
	
	int timecnt;
	int sockerr;

	printf( "Waiting for connect...\n" );
  	if ((m_Fd = accept(m_FdAcpt, (struct sockaddr *)&m_Addr, &len)) < 0){
    	printf( "accept() error\n" );
    	closesocket( m_FdAcpt );
		return -2;
  	}
  	printf( "Connected.\n" );

	sprintf(m_ClientIp,"%s",inet_ntoa(m_Addr.sin_addr));

  	exit_flag = 0;

    printf( "Waiting to receive data...\n" );
    recv_len = 0;  
	timecnt = 0;
	sockerr=0;
    while( 1 ){
      	do{
        	FD_ZERO( &r_socket );
        	FD_ZERO( &w_socket );
        	FD_ZERO( &e_socket );
        	FD_SET( m_Fd, &r_socket );

        	time.tv_sec  = 2;  
	      	time.tv_usec = 0;
        	err = select( m_Fd+1, &r_socket, &w_socket, &e_socket, &time );
        	if(err < 0){
          		printf( "select() error\n" );
          		exit_flag = 1;
          		break;
       		}
        	else if( err == 0 ){
          		printf( "(receive time out)\n" ); 
				return 0;
        	}
        	else if( FD_ISSET( m_Fd, &r_socket ) == 0 ){
          		printf( "FD_ISSET() error\n" );
          		exit_flag = 1;
          		break;
        	}
		}while( err == 0 );
		memset(revbuf,0,sizeof(revbuf));
		recv_len=0;
		while(1){
			err = recv( m_Fd, &revbuf[recv_len], sizeof(revbuf)-recv_len  , 0 );
			if( err == -1 ){  
				sockerr++;
				if(sockerr > 10){
					printf( "Err. Rcv Transfer failed.\n" );
					exit_flag = 1;
					return -1;
				}
				else{
					Sleep(100);
					continue;
				}
			}
			sockerr=0;


			recv_len += err;
			memcpy(buf,revbuf,5);
			buf[5]=0;
			sockerr=0;		

			if(atol(buf) <= recv_len-5){
				exit_flag = 0;
				break;
			}

			else{
				Sleep(100);
				timecnt++;
				if(timecnt > 10){		
					printf("err=%d:len=%ld:rcv=%s\n",err,recv_len,recvdat);
					exit_flag = 1;
					break;
				}
			}
		}

		if(atol(buf) <= recv_len-5) break;

	}

    if( exit_flag == 1 ){
		return -1;
	}
	memcpy(recvdat,revbuf+5,atol(buf));



	return atol(buf)-5;
}

int  TcpServer::Send(char *msg)
{
	return(this->Send(msg,strlen(msg),0));
}


int TcpServer::SetPortNo(int portNo)
{
	if(portNo>0x09999 || portNo <0x900) return(1);
	m_PortNo = portNo;
	
	return(0);	

}
int TcpServer::SetIPAdress(char *IPAdress)
{
	strncpy(m_IPAdress,IPAdress,15);
	m_IPAdress[15]=0;
	return(0);	

}

int TcpServer::SetPcName(char *PcName)
{
	strncpy(fPcName,PcName,20);
	fPcName[20]=0;
	return(0);	
}

int TcpServer::SetProgramName(char *ProgramName)
{
	strncpy(fProgramName,ProgramName,20);
	fProgramName[20]=0;
	return(0);	

}
int TcpServer::CheckStatus()
{
	return(m_Status);
}

int  TcpServer::Recv( char *recvdat,int bufsize )
{
	int		recv_len;
	int		err;
  	int len = sizeof(m_AddrAcpt);
	char buf[5+1];
	char revbuf[MAX_SENDLENGTH+10];
	int timeout,sockerr;

#ifdef _DEBUGPRINT
	printf( "\nWaiting for connect..." );
#endif
  	if ((m_Fd = accept(m_FdAcpt, (struct sockaddr *)&m_Addr, &len)) < 0){
    	printf( "accept() error\n" );
    	closesocket( m_FdAcpt );
		return -2;
  	}
#ifdef _DEBUGPRINT
  	printf( "Connected." );
#endif

	sprintf(m_ClientIp,"%s",inet_ntoa(m_Addr.sin_addr));

#ifdef _DEBUGPRINT
    printf( "\nWaiting to receive data..." );
#endif

   	FD_ZERO( &r_socket );
   	FD_ZERO( &w_socket );
   	FD_ZERO( &e_socket );
	FD_SET( m_Fd, &r_socket );
    time.tv_sec  = 1; 
    time.tv_usec = 0;
    err = select( m_Fd+1, &r_socket, &w_socket, &e_socket, &time );
    if(err < 0){
   		printf( "select() error\n" );
   		return(0);
	}
   	else if( err == 0 ){
	printf( "(receive time out)\n" );  
		return 0;
   	}
  	else if( FD_ISSET( m_Fd, &r_socket ) == 0 ){
		printf( "FD_ISSET() error\n" );
		return 0;
	}

    recv_len = 0; 
	timeout=0;
	sockerr=0;
	memset( revbuf,0,sizeof(revbuf));

    while( 1 ){

		err = recv( m_Fd, &revbuf[recv_len], sizeof(revbuf)-recv_len  , 0 );
	
		if( err == -1 ){ 
			if(++sockerr > 10){
				printf( "Rcv Transfer failed.\n" );
				return(0);
			}
			else{
				Sleep(100);
				continue;
			}
		}
		sockerr=0;

		recv_len += err;

		sockerr=0;		

		memcpy(buf,revbuf,5);
		buf[5]=0;

		if(atol(buf)<=recv_len-5) break;
		else{
			if(++timeout >= 30){		
				printf("err=%d:len=%ld:rcv=%s\n",err,recv_len,recvdat);
				return(0);
			}
			Sleep(100);
		}

#ifdef _DEBUGPRINT
		printf("err=%d:len=%ld:rcv=%s\n",err,recv_len,recvdat);
#endif
	}
	memcpy(recvdat,revbuf+5,atol(buf)> bufsize ? bufsize:atol(buf));

	return atol(buf);
}


int  TcpServer::Send(char *msg,int sendlen,DWORD close_wait)
{
  	int		exit_flag=0;	
	int		err;
	char buf[MAX_SENDLENGTH+10];

	sprintf(buf,"%05ld",sendlen);
	memcpy(buf+5, msg, sendlen);
	buf[sendlen+5]=0;
	
#ifdef _DEBUGPRINT
    printf( "\nWaiting to send data..." );
#endif
 	do{
	    FD_ZERO( &r_socket );
	    FD_ZERO( &w_socket );
	    FD_ZERO( &e_socket );
	    FD_SET( m_Fd, &w_socket );
	    time.tv_sec  = 1;  
	    time.tv_usec = 0;
	    err = select( m_Fd+1, &r_socket, &w_socket, &e_socket, &time );
	    if(err < 0){
	        printf( "select() error\n" );
	        exit_flag = 1;
	        break;
      	}
	    else if( err == 0 ){
	        printf( "(send time out)\n" );  
	    }
	    else if( FD_ISSET( m_Fd, &w_socket ) == 0 ){
	       	printf( "FD_ISSET() error\n" );
	        exit_flag = 1;
	        break;
	    }
	}while( err == 0 );

	err = send( m_Fd, buf, sendlen+5 , 0 );
    if( err == -1 ){
		printf( "Send Transfer failed.\n" );
    }
#ifdef _DEBUGPRINT
    printf( "Send Transfer complete.\n" );
#endif
    if(  exit_flag == 1 ) return -1;

	if( close_wait < 1 ){
		closesocket( m_Fd );
		return 0;
	}

	WSAEVENT	hEvent;	
	WSANETWORKEVENTS events;
	DWORD ret;
	hEvent = WSACreateEvent();
	if(hEvent == WSA_INVALID_EVENT)
	{
		closesocket(m_Fd);
        WSACleanup();
		return -4;
	}

	err = WSAEventSelect(m_Fd,hEvent,FD_CLOSE );
	if(err == SOCKET_ERROR)
	{
        printf("\nWSAEventSelect()Err: %d\n", err);
		WSACloseEvent(hEvent);
		closesocket(m_Fd);
		return -5;
	}

	ret=WSAWaitForMultipleEvents(1,&hEvent,FALSE,close_wait,FALSE);
	if (ret == WSA_WAIT_TIMEOUT)
    {
		WSACloseEvent(hEvent);
		closesocket(m_Fd);
        return(-1);
    }
    ret = WSAEnumNetworkEvents(m_Fd,hEvent,&events);
   if (ret == SOCKET_ERROR)
   {
		WSACloseEvent(hEvent);
		closesocket(m_Fd);
        printf("WSAEnumNetworkEvents():Err=%d\n",WSAGetLastError());
        return(-2);
   }

#ifdef _DEBUGPRINT
    if(events.lNetworkEvents & FD_CLOSE)
	{
		printf("Event close%d,\n",FD_CLOSE);
	}
	getchar();
#endif

	WSACloseEvent(hEvent);
	closesocket( m_Fd );
	memset(m_ClientIp,0,sizeof(m_ClientIp));
	return 0;
}

int  TcpServer::RecvDirect( char *recvdat,int bufsize ,int time_out/*=1*/)
{
	int		recv_len;
	int		err;
  	int len = sizeof(m_AddrAcpt);
	int timeout,sockerr;
	char buf[10];

#ifdef _DEBUGPRINT
	printf( "\nWaiting for connect..." );
#endif
  	if ((m_Fd = accept(m_FdAcpt, (struct sockaddr *)&m_Addr, &len)) < 0){
    	printf( "accept() error\n" );
    	closesocket( m_FdAcpt );
		return -2;
  	}
#ifdef _DEBUGPRINT
  	printf( "Connected." );
#endif

	sprintf(m_ClientIp,"%s",inet_ntoa(m_Addr.sin_addr));

#ifdef _DEBUGPRINT
    printf( "\nWaiting to receive data..." );
#endif

   	FD_ZERO( &r_socket );
   	FD_ZERO( &w_socket );
   	FD_ZERO( &e_socket );
	FD_SET( m_Fd, &r_socket );

    time.tv_sec  = time_out;  
    time.tv_usec = 0;
    err = select( m_Fd+1, &r_socket, &w_socket, &e_socket, &time );
    if(err < 0){
   		printf( "select() error\n" );
   		return(-1);
	}
   	else if( err == 0 ){
	printf( "(receive time out)\n" ); 
		return -2;
   	}
  	else if( FD_ISSET( m_Fd, &r_socket ) == 0 ){
		printf( "FD_ISSET() error\n" );
		return -3;
	}

    recv_len = 0; 
	timeout=0;
	sockerr=0;
	memset( recvdat,0,bufsize);
	int chk=0,pos=0,dlen=0;

    while( 1 ){
		err = recv( m_Fd, &recvdat[recv_len], bufsize-recv_len  , 0 );
		if( err == -1 ){ 
			if(++sockerr > 10){
				printf( "Rcv Transfer failed.\n" );
				return(0);
			}
			else{
				Sleep(50);
				continue;
			}
		}

		recv_len += err;
		sockerr=0;		

		if(!chk){
			memcpy(buf,recvdat,5);
			buf[5]=0;
			dlen=atol(buf);
			chk=1;
		}


		if(dlen<=recv_len-5) break;
		else{
			if(++timeout >= 30){		
				printf("err=%d:len=%ld:rcv=%s\n",err,recv_len,recvdat);
				return(-10);
			}
			Sleep(100);
		}

#ifdef _DEBUGPRINT
		printf("err=%d:len=%ld:rcv=%s\n",err,recv_len,recvdat);
#endif
	}

	return recv_len;
}


int  TcpServer::SendDirect(char *msg,int sendlen,DWORD close_wait)
{
  	int		exit_flag=0;		
	int		err;

#ifdef _DEBUGPRINT
    printf( "\nWaiting to send data..." );
#endif
 	do{

	    FD_ZERO( &r_socket );
	    FD_ZERO( &w_socket );
	    FD_ZERO( &e_socket );
	    FD_SET( m_Fd, &w_socket );
	    time.tv_sec  = 1;  
	    time.tv_usec = 0;
	    err = select( m_Fd+1, &r_socket, &w_socket, &e_socket, &time );
	    if(err < 0){
	        printf( "select() error\n" );
	        exit_flag = 1;
	        break;
      	}
	    else if( err == 0 ){
	        printf( "(send time out)\n" ); 
	    }
	    else if( FD_ISSET( m_Fd, &w_socket ) == 0 ){
	       	printf( "FD_ISSET() error\n" );
	        exit_flag = 1;
	        break;
	    }
	}while( err == 0 );


	err = send( m_Fd, msg, sendlen, 0 );
    if( err == -1 ){
		printf( "Send Transfer failed.\n" );
    }
#ifdef _DEBUGPRINT
    printf( "Send Transfer complete.\n" );
#endif
    if(  exit_flag == 1 ) return -1;

	WSAEVENT	hEvent;		
	WSANETWORKEVENTS events;
	DWORD ret;
	hEvent = WSACreateEvent();
	if(hEvent == WSA_INVALID_EVENT)
	{
		closesocket(m_Fd);
        WSACleanup();
		return -4;
	}

	err = WSAEventSelect(m_Fd,hEvent,FD_CLOSE );
	if(err == SOCKET_ERROR)
	{
        printf("\nWSAEventSelect()Err: %d\n", err);
		WSACloseEvent(hEvent);
		closesocket(m_Fd);
		return -5;
	}

	if( close_wait < 1 ) close_wait=1000;
	ret=WSAWaitForMultipleEvents(1,&hEvent,FALSE,close_wait,FALSE);
	if (ret == WSA_WAIT_TIMEOUT)
    {
		WSACloseEvent(hEvent);
		closesocket(m_Fd);
        return(-1);
    }
    ret = WSAEnumNetworkEvents(m_Fd,hEvent,&events);
   if (ret == SOCKET_ERROR)
   {
		WSACloseEvent(hEvent);
		closesocket(m_Fd);
        printf("WSAEnumNetworkEvents():Err=%d\n",WSAGetLastError());
        return(-2);
   }


#ifdef _DEBUGPRINT
    if(events.lNetworkEvents & FD_CLOSE)
	{
		printf("Event close%d,\n",FD_CLOSE);
	}
	getchar();
#endif

	WSACloseEvent(hEvent);
	closesocket( m_Fd );

	return 0;
}
