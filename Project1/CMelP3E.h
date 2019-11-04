#pragma comment(lib,"ws2_32")
#ifndef MELSEC_P3E_H
#define MELSEC_P3E_H

#define  _WINSOCKAPI_
#include <windows.h>


#define UDP_MELPORT 5000;
#define DEV_WORD 0
#define DEV_BIT 1

#define P3E_FIX_DATALEN 24


class CMelsecP3E
{
public:
	CMelsecP3E();
	~CMelsecP3E();
	
	BOOL Init();

	void SetMode(BOOL binaryMode);

	int P3EWrite(char* ipaddr, unsigned short port, int devtype, char* devaddr, short counts, char* data);

	int P3EWrite(char* ipaddr, unsigned short port, int devtype, char* devaddr, short counts, char* data, BOOL binary);

	int P3EWriteA(char* ipaddr, unsigned short port, int devtype, char* devaddr, short counts, char* data);
	int P3EWriteB(char* ipaddr, unsigned short port, int devtype, char* devaddr, short counts, char* data);


	int UdpSendA(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize);
	int UdpSendB(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize);


	int P3ERead(char* ipaddr, unsigned short port, char* buf, int bufsize, int devtype, char* devaddr, short counts);
	int P3ERead(char* ipaddr, unsigned short port, char* buf, int bufsize, int devtype, char* devaddr, short counts, BOOL binary);

	int P3EReadA(char* ipaddr, unsigned short port, char* buf, int bufsize, int devtype, char* devaddr, short counts);

	int P3EReadB(char* ipaddr, unsigned short port, char* buf, int bufsize, int devtype, char* devaddr, short counts);

	int UdpSendBit(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize, short counts);

	unsigned short htoi(char *hexstr, short len);
	int foo(char * input, char * output);
	int P3EReadAscii(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts, int modeBinaryOrAscii);
private:
	BOOL m_fOpen;
	BOOL m_fBinMode;
};
#endif
