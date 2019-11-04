#pragma once
#ifndef COM_PLC4E_TCP_H
#define COM_PLC4E_TCP_H

#include <winsock2.h>
#include <windows.h>

#ifndef CR
#define CR 0x0d
#endif
#ifndef LF
#define LF 0x0a
#endif
#ifndef SOH
#define SOH 0x01
#endif
#ifndef STX
#define STX 0x02
#endif
#ifndef ETX
#define ETX 0x03
#endif
#ifndef EOT
#define EOT 0x04
#endif
#ifndef ENQ
#define ENQ 0x05
#endif
#ifndef ACK
#define ACK 0x06
#endif
#ifndef NAK
#define NAK 0x15
#endif


// Ad race + transmission header length other than data section ENQ (1) + ST (2) + PC (2) + CMD (2) + WAIT (1) + SUM (2) + number of devices (2) + CRLF (2)
const int P4_HEADLEN = 14;

class CMelsecP4TCP
{
public:
	CMelsecP4TCP(void);
	~CMelsecP4TCP(void);

	BOOL Init(int wait);

	// Specified number of word device read & write
	int P4WriteW(char* ip, unsigned short port, char* station, char* pcstation, char* addr, short counts, char* msg);
	int P4ReadW(char* ip, unsigned short port, char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts);

	// 1 bit device read & write
	int P4WriteB(char* ip, unsigned short port, char* station, char* pcstation, char* addr, int onoff);
	int P4ReadB(char* ip, unsigned short port, char* station, char* pcstation, char* addr);

	// Multipoint bit device read & write
	int P4ReadBB(char* ip, unsigned short port, char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts);
	int P4WriteBB(char* ip, unsigned short port, char* station, char* pcstation, char* addr, int cnt, char* dat); // 2012.06.20 coba

	int P2C4WriteW(char* ip, unsigned short port, char* station, char* pcstation, char* addr, short counts, char* msg);
	int P2C4ReadW(char* ip, unsigned short port, char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts);


	// String conversion
	int Str2Mel(char* buf, unsigned int bufsize, char* str);
	int Mel2Str(char* buf, unsigned int bufsize, char* melstr);
	int UStr2Mel(char* buf, unsigned int bufsize, unsigned char* str);

	// Upper / lower word replacement (BCD 8-digit conversion)
	BOOL ULChg(char* buf);

protected:

private:
	BOOL m_fOpen;					// Open = TRUE, close = FALSE
	int m_Wait;

	// Calculation of sum check
	int GetCheckSum(char* str);

	// Create command
	void MakeCmd(char* buf, int bufsize, char* station, char* pc, char* cmd, char* addr, short counts, char*msg);
	void MakeCmd2C(char* buf, int bufsize, char* station, char* pc, char* cmd, char* addr, short counts, char*msg);

	// Internally used transmit and receive functions
	int TcpSend(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize);
	int Answer(char* ip, unsigned short port, char flg, char* station, char* pc);

	// Transmit / receive buffer
	char m_buf[4096];

	unsigned char htoi(char *hexstr, short len);

};

#endif