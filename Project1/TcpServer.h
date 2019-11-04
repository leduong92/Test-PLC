#ifndef TCPServer_H
#define TCPServer_H


#include <winsock2.h>


#define DEF_TRANSPORT_NO	26578         

#define MAX_SENDLENGTH	8192//20480
#define MAX_RECVLENGTH	8192//20480

#define SHORT_SENDLENGTH	254
#define SHORT_RECVLENGTH	254

class TcpServer
{

	char	fPcName[30];
	char	fProgramName[30];
	int	 	fCommand;
	long 	fRcvPacketNo,fSendPaketNo;
	int  	m_PortNo;
	char 	m_IPAdress[20];
	char 	m_ClientIp[20];	//client ip

	int m_Status;
  	struct sockaddr_in  m_AddrAcpt;
  	SOCKET              m_FdAcpt;
  	struct sockaddr_in  m_Addr;
  	SOCKET              m_Fd;
  	WSADATA             WSAData;
    WSAEVENT			m_Event;

  	struct timeval      time;
  	fd_set              r_socket, w_socket, e_socket;

public:

			 TcpServer(int port);
			 ~TcpServer();

	 TcpServer();
	int Initialize(int PortNo = DEF_TRANSPORT_NO,int PortCheck=TRUE);
	int Send( char *message,int sendlen,DWORD close_wait);	
	int Recv( char *msg,int len);


	virtual BOOL ITcpServer(void);
	virtual int CheckStatus(void);

	virtual int Send( char *message);
	virtual int Recv( char *msg);
    virtual int SetPortNo(int PortNo);
	virtual int SetIPAdress(char *IPAdress);
	virtual int SetPcName(char *PcName);
	virtual int SetProgramName(char *ProgramName);
	int Wait( DWORD timeout=5000);		

	int SendDirect( char *message,int sendlen,DWORD close_wait);
	int RecvDirect( char *msg,int len,int time_out=1);	
	void GetClientIp( char *ip );	
protected:

};
#endif
