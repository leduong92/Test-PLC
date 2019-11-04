#ifndef KEYENCE_LV21A_H
#define KEYENCE_LV21A_H

#include <winsock2.h>
#include <windows.h>

//#define UDP_MELPORT 5000


#define DEV_WORD 0
#define DEV_BIT  1

//#define P3E_FIX_DATALEN 24

class CKeyL2AE
{
	public:
		CKeyL2AE( void );
		~CKeyL2AE( void );

		BOOL Init(void);

		//(2C)
		int L2AWrite(char* ipaddr,unsigned short port,int devtype,char* devaddr, short counts, char* data);
		int L2ARead( char* ipaddr,unsigned short port,char* buf,int bufsize,int devtype,char* devaddr, short counts);

		int L2ASetBit(char* ipaddr,unsigned short port,char* devaddr,BOOL on );

		int GetPlcType(char* ipaddr,unsigned short port,char* recvBuf);

		int Str2Mel(char* buf, unsigned int bufsize, char* str);
		int Mel2Str(char* buf, unsigned int bufsize, char* melstr);

		BOOL ULChg( char* buf );

		unsigned short htoi( char *hexstr , short len );
	protected:

	private:
		BOOL m_fOpen;				

		int UdpSend( char* ipaddr,unsigned short port,char* data,int size,char* rcv,int rsize);


};
#endif
