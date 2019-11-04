#pragma once
#ifndef MELSEC_P1E_H
#define MELSEC_P1E_H


#define  _WINSOCKAPI_
#include <windows.h>

// Melsec default port number (only available for binary communication)
#define UDP_MELPORT 5000

// type of device
#define DEV_WORD 0
#define DEV_BIT 1

// Fixed part of transmission data length h
// CPU monitoring timer (4) + command (4) + sub command (4) + device code (2)
// + Dev code (2) + top device (6) + points (2)
#define P3E_FIX_DATALEN 24

class CMelsecP1E
{
public:
	CMelsecP1E();
	~CMelsecP1E();

	BOOL Init(BOOL tcp = FALSE);


	// The specified number of word device read & write (2C mode)
	int P1EWrite(char * ipaddr, unsigned short port, int devtype, char * devname, int devaddr, short counts, char * data);
	int P1ERead(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devname, int devaddr, short counts);

	// String conversion
	int Str2Mel(char * buf, unsigned int bufsize, char * str);
	int Mel2Str(char * buf, unsigned int bufsize, char * melstr);

	// High-order / low-order word exchange (BCD 8-digit conversion)
	BOOL ULChg(char * buf);

private:
	BOOL m_fOpen;				
						
	BOOL m_tcp;
	int TcpSend(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize);

	int UdpSend(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize);

	unsigned char htoi(char *hexstr, short len);
};

#endif