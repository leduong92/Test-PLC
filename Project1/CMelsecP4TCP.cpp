#define _WINSOCK_DEPRECATED_NO_WARNINGS
// ============================================================ =========================================================
// Melsec protocol 4 format communication onTCP / IP
// Send communication contents on TCP for server mode of Serial-Ethernet converter
// Sum check, xon / off control not available.
// ============================================================ =========================================================
#include "CMelsecP4TCP.h"
#include <stdio.h>


// Function: Constructor. Create data buffer with default size
CMelsecP4TCP::CMelsecP4TCP(void)
{
	m_fOpen = FALSE;
}

CMelsecP4TCP::~CMelsecP4TCP()
{
	WSACleanup();
}


// ============================================================ ============================================================= =======
// BOOL CMelsecP4TCP :: Init (void)
// type: private
// Function: Initialization
// Return value: TRUE or FALSE
// ============================================================ ============================================================= =======
BOOL CMelsecP4TCP::Init(int wait)
{
	// OK while open
	if (m_fOpen) return(TRUE);

	WSADATA			WSAData;

	m_Wait = wait;

	// Socket initialization
	if (WSAStartup(MAKEWORD(2, 2), &WSAData)) return(-1);


	m_fOpen = TRUE;

	return(TRUE);
}

// ================================================================================================================================
// int CMelsecP4TCP :: P4ReadW (char * buf, int bufsize, char * station, char * pcstation, char * addr, short counts)
// Function: Receive word data block in protocol 4 format
// Argument 1: Receive buffer
// Argument 2: Receive buffer size
// Argument 3: Station number
// Argument 4: PC station number
// Argument 5: Reception start address
// Argument 6: Number of received blocks (max 64)
// type: public
// ================================================================================================================================
int CMelsecP4TCP::P4ReadW(char* ipaddr, unsigned short port, char* buf, int bufsize	, char* station, char* pcstation, char* addr, short counts)
{
	int ret, len;
	int retry = 0, sum;
	char data[10], txt[10], rcv[1024];
	char initcom[5];

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 64) return(-2);

	memset(data, 0, sizeof(data));
	// Transmission loop
	while (retry < 8) {
		if (retry) {
			ret = TcpSend(ipaddr, port, initcom, 3, NULL, 0);
		}
		//---- Creation of transmission string ------
		// Normal command
		if (strlen(addr) == 5)
			MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "WR", addr, counts, "");
		// Extended command (D1023 or later compatible)
		else MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "QR", addr, counts, "");

		//----- Data transmission and reception ---
		len = TcpSend(ipaddr, port, m_buf, strlen(m_buf), rcv, sizeof(rcv));
		if (len < 1) {
			printf("len=%d", len);
			return(-3);
		}
		// Determine the remaining number of processes for each command
		//----- In the case of STX (data arrival) ------
		if (rcv[0] == STX) {
			//There is at least 14 bytes in 1 register read
			if (len < 14) {
				retry++;									// retry
				continue;
			}

			// Copy the first STX and the last four (sum check crlf)
			memset(m_buf, 0, sizeof(m_buf));
			memcpy(m_buf, rcv + 1, len - 5);

			// Copy the sum check section
			memcpy(data, rcv + len - 4, 2);

			// Compare sum check
			sum = GetCheckSum(m_buf);
			wsprintf(txt, "%02X", sum);

			// In case of OK, return ACK and end normally
			if (!memcmp(txt, data, 2)) {
				Answer(ipaddr, port, ACK, station, pcstation);
				break;
			}
			// When NG, returns NAK
			Answer(ipaddr, port, NAK, station, pcstation);
			retry++;										
			continue;
		}

		//----- In the case of NAK ------
		else if (rcv[0] == NAK) {
			retry++;										
			continue;
		}

		//----- It is an error. -------
		else return(-8);
	}
	if (retry >= 8) return(-9);

	// Remove ETX and copy received data to buffer
	memcpy(buf, m_buf + 4, (int)strlen(m_buf + 4) - 1 > bufsize ? bufsize : strlen(m_buf + 4) - 1);

	return(0);
}

int CMelsecP4TCP::P4WriteW(char* ipaddr, unsigned short port, char* station, char* pcstation, char* addr, short counts, char* msg)
{
	int ret, len;
	int retry = 0;
	char data[128];
	char initcom[5];

	// not open error
	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 64) return(-2);

	// Length over
	if (strlen(addr) + strlen(msg) + P4_HEADLEN >= sizeof(m_buf)) return(-10);

	if (counts * 4 != (int)strlen(msg)) return(-11);

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;


	// Create send string
	// Normal command
	if (strlen(addr) == 5)
		MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "WW", addr, counts, msg);
	// extended command (D 1023 or later compatible)
	else
		MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "QW", addr, counts, msg);

	// semd-loop
	while (retry < 8) {
		if (retry) {
			ret = TcpSend(ipaddr, port, initcom, 3, NULL, 0);
		}
		//----- Data transmission and reception ---
		len = TcpSend(ipaddr, port, m_buf, strlen(m_buf), data, sizeof(data));
		if (len < 1) {
			if (retry++ < 8)continue;
			return(-3);
		}
		// Determine the number of remaining bytes for each command
		if (data[0] == ACK)  break;		// in case of ACK
		else {
			if (retry++ < 8)continue;
			return(-4);						// it's an error.
		}
	}
	// Even if the number of retries is over, it returns 0
	if (retry >= 8) return(-9);
	return(0);
}

int CMelsecP4TCP::P4ReadB(char* ipaddr, unsigned short port, char* station, char* pcstation, char* addr)
{
	int ret, buf;
	char data[20];

	memset(data, 0, sizeof(data));
	ret = P4ReadBB(ipaddr, port, data, sizeof(data), station, pcstation, addr, 1);
	if (ret) return(ret);

	//Copy received data to buffer
	buf = data[0] - 0x30;

	return(buf);
}

int CMelsecP4TCP::P4ReadBB(char* ipaddr, unsigned short port, char* buf, int bufsize
	, char* station, char* pcstation, char* addr, short cnt)
{
	int ret, len;
	int retry = 0, sum;
	char data[10], txt[10], rcv[1024];
	char initcom[5];

	if (m_fOpen == FALSE) return(-1);

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	memset(data, 0, sizeof(data));
	// Transmission loop
	while (retry < 8) {
		if (retry) {
			ret = TcpSend(ipaddr, port, initcom, 3, NULL, 0);
		}

		//---- Creation of transmission string ------
		MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "BR", addr, cnt, "");

		//----- Data transmission and reception ---
		len = TcpSend(ipaddr, port, m_buf, strlen(m_buf), rcv, sizeof(rcv));
		if (len < 1) return(-3);

		// Determine the remaining number of processes for each command
		// --- In the case of STX (data arrival) ------
		if (rcv[0] == STX) {
			// There is at least 11 bytes in 1 bit read
			if (len < 11) {
				retry++;									
				continue;
			}
			// Copy the first STX and the last four (sum check crlf)
			memset(m_buf, 0, sizeof(m_buf));
			memcpy(m_buf, rcv + 1, len - 5);

			// Copy the sum check section
			memcpy(data, rcv + len - 4, 2);

			// Compare sum check
			sum = GetCheckSum(m_buf);
			wsprintf(txt, "%02X", sum);

			//In case of OK, return ACK and end normally
			if (!memcmp(txt, data, 2)) {
				Answer(ipaddr, port, ACK, station, pcstation);
				break;
			}
			//When NG, returns NAK
			Answer(ipaddr, port, NAK, station, pcstation);
			retry++;										
			continue;
		}

		else if (rcv[0] == NAK) {
			retry++;										
			continue;
		}

		else return(-8);
	}

	// Even if the number of retries is over, it returns 0
	if (retry >= 8) return(-9);

	//ETX cut and copy received data to buffer
	memcpy(buf, m_buf + 4, (int)(strlen(m_buf + 4) - 1) > bufsize ? bufsize : strlen(m_buf + 4) - 1);

	return(0);
}

int CMelsecP4TCP::P4WriteB(char* ipaddr, unsigned short port, char* station, char* pcstation, char* addr, int onoff)
{
	int ret;
	char send[12];

	// Create a send string
	if (!onoff) strcpy(send, "0\0");
	else strcpy(send, "1\0");				// off

	ret = P4WriteBB(ipaddr, port, station, pcstation, addr, 1, send);
	return(ret);
}

int CMelsecP4TCP::P4WriteBB(char* ipaddr, unsigned short port, char* station, char* pcstation, char* addr, int cnt, char* dat)
{
	int ret, len;
	int retry = 0;
	char data[128];
	char initcom[5];


	if (m_fOpen == FALSE) return(-1);

	// Check because the number of registers to set = number of data bytes
	if ((int)strlen(dat) != cnt) return(-2);

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "BW", addr, cnt, dat);

	// semd-loop
	while (retry < 8) {
		if (retry) {
			ret = TcpSend(ipaddr, port, initcom, 3, NULL, 0);
		}

		//----- Data transmission and reception ---
		len = TcpSend(ipaddr, port, m_buf, strlen(m_buf), data, sizeof(data));
		if (len < 1) {
			if (retry++ < 8)continue;
			return(-3);
		}

		// Determine the number of remaining bytes for each command
		if (data[0] == ACK) break;		// In the case of ACK
		else retry++;
	}

	if (retry >= 8) return(-9);
	return(0);
}


int CMelsecP4TCP::P2C4ReadW(char* ipaddr, unsigned short port, char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts)
{
	int ret, len;
	int retry = 0, sum;
	char rcv[4096], data[10], txt[10];
	char initcom[5];

	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 960) return(-2);

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	// semd-loop
	while (retry <= 8) {
		if (retry) {
			ret = TcpSend(ipaddr, port, initcom, 3, NULL, 0);
		}

		// Create send string (2C)
		MakeCmd2C(m_buf, sizeof(m_buf), station, pcstation, "2", addr, counts, "");

		len = TcpSend(ipaddr, port, m_buf, strlen(m_buf), rcv, sizeof(rcv));
		if (len < 1) return(-3);

		// Determine the remaining number of processes for each command
		// --- In the case of STX (data arrival) ------
		if (rcv[0] == STX) {
			// At least 16 bytes in 1 register read
			if (len < 16) {
				retry++;										
				continue;
			}

			// Copy the first STX and the last four (sum check crlf)
			memset(m_buf, 0, sizeof(m_buf));
			memcpy(m_buf, rcv + 1, len - 5);

			memcpy(data, rcv + len - 4, 2);

			sum = GetCheckSum(m_buf);
			wsprintf(txt, "%02X", sum);

			if (!memcmp(txt, data, 2)) break;
		}
		else retry++;
	}
	if (retry >= 8) return(-9);

	memcpy(buf, m_buf + 6, (int)(strlen(m_buf + 6) - 1) > bufsize ? bufsize : strlen(m_buf + 6) - 1);

	return(0);


}

// Function: Send word block in protocol 4 format
int CMelsecP4TCP::P2C4WriteW(char* ipaddr, unsigned short port, char* station, char* pcstation, char* addr, short counts, char* msg)
{
	int ret, len;
	int retry = 0;
	char data[20];
	char initcom[5];

	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 960) return(-2);

	// Length over
	if (strlen(addr) + strlen(msg) + P4_HEADLEN >= sizeof(m_buf)) return(-10);

	if (counts * 4 != (int)strlen(msg)) return(-11);

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	// Create send string (2C)
	MakeCmd2C(m_buf, sizeof(m_buf), station, pcstation, "4", addr, counts, msg);

	// semd-loop
	while (retry <= 8) {
		if (retry) {
			ret = TcpSend(ipaddr, port, initcom, 3, NULL, 0);
		}

		len = TcpSend(ipaddr, port, m_buf, strlen(m_buf), data, sizeof(data));
		if (len < 1) {
			if (retry++ < 8)continue;
			return(-3);
		}
		if (data[0] == ACK)  break;		
		else {
			if (retry++ < 8)continue;
			return(-4);						
		}
	}
	if (retry >= 8) return(-9);
	return(0);
}



// ============================================================ ============================================================= =======
// Function: Reply to PLC
// type: private
// Argument 1: ACK or NAK
// Argument 2: Station number
// Argument 3: PC station number
// ============================================================ ============================================================= =======
int CMelsecP4TCP::Answer(char* ip, unsigned short port, char flg, char* station, char* pc)
{
	char buf[10];
	int ret;

	memset(buf, 0, sizeof(buf));

	buf[0] = flg;
	memcpy(buf + 1, station, 2);
	memcpy(buf + 3, pc, 2);
	buf[5] = CR;
	buf[6] = LF;

	ret = TcpSend(ip, port, buf, strlen(buf), NULL, 0);
	return(0);
}

// ============================================================ ============================================================= =======
// void CMelsecP4TCP :: MakeCmd (char * buf, char * station, char * pc, char * cmd, char * addr, short counts, char * msg)
// Function: Create send data in protocol 4 format
// Argument 1: Creation buffer
// Argument 2: Creation buffer size
// Argument 3: Station number
// Argument 4: PC station number
// Argument 5: Command
// Argument 6: Start address
// Argument 7: Number of device blocks (max 64)
// type: public
// ============================================================ ============================================================= =======
void CMelsecP4TCP::MakeCmd(char* buf, int bufsize, char* station, char* pc, char* cmd, char* addr, short counts, char*msg)
{
	char txt[10];
	int sum;

	memset(buf, 0, bufsize);
	buf[0] = ENQ;

	// Station number, PC station number
	strcat(buf, station);
	strcat(buf, pc);

	// command
	strcat(buf, cmd);

	// weight
	wsprintf(txt, "%d", m_Wait);
	memcpy(&buf[strlen(buf)], txt, 1);

	//Top device
	strcat(buf, addr);

	// Device score
	wsprintf(txt, "%02X", counts);
	memcpy(&buf[strlen(buf)], txt, 2);

	// Send message
	strcat(buf, msg);

	// sum check
	sum = GetCheckSum(&buf[1]);
	wsprintf(txt, "%02X", sum);
	memcpy(&buf[strlen(buf)], txt, 2);

	buf[strlen(buf)] = CR;
	buf[strlen(buf)] = LF;
}

void CMelsecP4TCP::MakeCmd2C(char* buf, int bufsize, char* station, char* pc, char* cmd, char* addr, short counts, char*msg)
{
	char txt[10];
	int sum;

	memset(buf, 0, bufsize);
	buf[0] = ENQ;
	buf[1] = 'F';
	buf[2] = 'B';


	// Station number, PC station number
	strcat(buf, station);
	strcat(buf, pc);

	// command
	strcat(buf, cmd);


	//Leading device (8) Ex: ZR123456
	strcat(buf, addr);

	// Device score (4)
	wsprintf(txt, "%04X", counts);
	memcpy(&buf[strlen(buf)], txt, 4);

	// Send message
	strcat(buf, msg);

	sum = GetCheckSum(&buf[1]);
	wsprintf(txt, "%02X", sum);
	memcpy(&buf[strlen(buf)], txt, 2);

	buf[strlen(buf)] = CR;
	buf[strlen(buf)] = LF;
}


int CMelsecP4TCP::GetCheckSum(char* str)
{
	int data = 0;
	int i, len;

	if (str == NULL) return(0);

	len = strlen(str);
	if (len <= 0) return(0);

	for (i = 0; i< len; i++)	data += (int)str[i];

	data = data & 0x000000ff;

	return(data);
}

int CMelsecP4TCP::Str2Mel(char* buf, unsigned int bufsize, char* str)
{
	unsigned int i, len;
	char txt[10];

	len = strlen(str);
	if (len * 2 >= bufsize) return(-1);

	memset(buf, 0, bufsize);

	for (i = 0; i<len; i++) {
		if (i + 1 == len)
			wsprintf(txt, "00");
		else
			wsprintf(txt, "%02X", (unsigned char)str[i + 1]);		
																	
		memcpy(&buf[i * 2], txt, 2);

		wsprintf(txt, "%02X", (unsigned char)str[i]);		
														
		i++;
		memcpy(&buf[i * 2], txt, 2);
	}

	return(0);

}

int CMelsecP4TCP::Mel2Str(char* buf, unsigned int bufsize, char* melstr)
{
	unsigned int i, len;

	len = strlen(melstr);
	if (bufsize <= len / 2) return(-1);

	memset(buf, 0, bufsize);

	for (i = 0; i<len; i += 2) {
		if (i + 2 >= len)
			buf[strlen(buf)] = 0x20;
		else
			buf[strlen(buf)] = htoi(&melstr[i + 2], 2);

		buf[strlen(buf)] = htoi(&melstr[i], 2);
		i += 2;
	}
	return(0);
}

// ============================================================ ============================================================= =======
// unsigned char CMelsecP4TCP :: htoi (char * hexstr, short len)
// Function: Convert hexadecimal (only 2 characters string) to integer value (unsigned char type)
// Argument: Conversion source data storage pointer
// Return value: Converted data
// ============================================================ ============================================================= =======
unsigned char CMelsecP4TCP::htoi(char *hexstr, short len)
{
	unsigned char ret = 0;
	short i;

	if (len > 4) return 0;
	for (i = 0; i < len; i++) {
		ret *= 16;
		if (hexstr[i] < 'A') ret += hexstr[i] - '0';
		else
			if (hexstr[i] < 'a') ret += hexstr[i] - 'A' + 10;
			else
				ret += hexstr[i] - 'a' + 10;
	}
	return(ret);
}
// ============================================================ ============================================================= =======
// int CMelsecP4TCP :: Str2Mel (char * buf, unsigned int bufsize, char * str)
// Function: Converts a character string to Melsec transmission format
// ex) "A1234"-> "31413433"
// Argument 1: Output buffer
// Argument 2: Buffer size (twice the conversion source data length + 1 is necessary)
// Argument 3: conversion source string data (NULL-terminated)
// Return value: 0 = OK
// ============================================================ ============================================================= =======
int CMelsecP4TCP::UStr2Mel(char* buf, unsigned int bufsize, unsigned char* str)
{
	unsigned int i, len;
	char txt[10];

	len = strlen((char*)str);
	if (len * 2 >= bufsize) return(-1);

	memset(buf, 0, bufsize);

	for (i = 0; i<len; i++) {
		if (i + 1 == len)
			wsprintf(txt, "00");
		else
			wsprintf(txt, "%02X", (unsigned)str[i + 1] & 0xFF);
		memcpy(&buf[i * 2], txt, 2);

		wsprintf(txt, "%02X", (unsigned)str[i] & 0xFF);
		i++;
		memcpy(&buf[i * 2], txt, 2);
	}

	return(0);

}


BOOL CMelsecP4TCP::ULChg(char* buf)
{
	char tmp[8];

	if (strlen(buf)<8)
		return FALSE;

	memcpy(tmp, buf, 8);
	memcpy(buf + 0, tmp + 4, 4);
	memcpy(buf + 4, tmp + 0, 4);

	return TRUE;
}

// ============================================================ ============================================================= =======
// int CMelsecP4TCP :: TcpSend (char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize)
// type: private
// Function: UDP transmission / reception
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: Transmission data
// Argument 4: Transmission data size
// Argument 5: Receive buffer
// Argument 6: Receive buffer size
// Return value: 0 = OK, less than 0 = network error, 1 = received data error
// ============================================================ ============================================================= =======
int CMelsecP4TCP::TcpSend(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize)
{
	unsigned long laddr;
	SOCKET soc;
	struct sockaddr_in rmthost;
	int ret;
	unsigned long arg;
	int len, len2, dlen;
	int wait_cnt, chk;
	fd_set r_socket;
	struct timeval      time;
	char rbuf[4096];

	//added to start in the right way

	//IP address conversion (to 32 bit)
	if (INADDR_NONE == (laddr = inet_addr(ipaddr)))return(-101);

	// Check send size
	if (size < 1) return(-103);

	soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (soc == INVALID_SOCKET) return(-104);

	// Create host socket structure input data
	// Store destination address (remote host) address in sockaddr structure
	memset(&rmthost, 0, sizeof(rmthost));			// zero clear
	rmthost.sin_family = AF_INET;						// PF_INET
	rmthost.sin_port = htons((unsigned short)port);		// port number
														// IP address
	rmthost.sin_addr.s_addr = laddr;				// IP address

	ret = connect(soc, (struct sockaddr*)&rmthost, sizeof(struct sockaddr));
	if (ret == SOCKET_ERROR) {
		printf("WLAN=port=%d\n", WSAGetLastError());
		closesocket(soc);
		Sleep(30);	
		return -105;										// Connect failed
	}


	arg = 0;
	if ((ioctlsocket(soc, FIONBIO, &arg)) != 0) {
		closesocket(soc);
		Sleep(30);	
		return -106;										// ioctl failure
	}


	FD_ZERO(&r_socket);
	FD_SET(soc, &r_socket);
	//    time.tv_sec  = 3;  / * Timeout 1 second * /
	time.tv_sec = 2000;	// Changed so that transmission timeout can be set
	time.tv_usec = 0;
	ret = select(1, NULL, &r_socket, NULL, &time);
	if (ret == SOCKET_ERROR) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);
		return -107;										// Data transmission failure
	}
	else if (ret == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -108;										// Data transmission failure
	}
	else if (FD_ISSET(soc, &r_socket) == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -109;										// Data transmission failure
	}


	len = size;
	len2 = 0;
	wait_cnt = 0;
	do {
		ret = send(soc, &data[len2], len - len2, 0);
		if (ret == SOCKET_ERROR) {
			shutdown(soc, 2);
			closesocket(soc);
			Sleep(30);	
			return -110;									// Data transmission failure
		}
		// Number of bytes sent
		len2 += ret;
		if (len != len2) {
			if (++wait_cnt > 4000) {
				shutdown(soc, 2);
				closesocket(soc);
				Sleep(30);	
				return -111;										// Data transmission failure
			}
			Sleep(5);
		}
		// wait timer
		else wait_cnt = 0;
	} while (len != len2);				// until it is sent

										// no reception
	if (rcv == NULL) {
		shutdown(soc, 2);							// 2: SHUT_RDWR
		closesocket(soc);
		return(0);
	}


	FD_ZERO(&r_socket);
	FD_SET(soc, &r_socket);

	time.tv_sec = 2000;// Changed to set reception timeout
	time.tv_usec = 0;
	ret = select(1, &r_socket, NULL, NULL, &time);
	if (ret == SOCKET_ERROR) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -112;										// Data transmission failure
	}
	else if (ret == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -113;										// Data transmission failure
	}
	else if (FD_ISSET(soc, &r_socket) == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);
		return -114;									// Data transmission failure
	}

	// still need a loop
	len = 0, len2 = 0, wait_cnt = 0, dlen = 0, chk = 0;
	memset(rbuf, 0, sizeof(rbuf));
	while (1) {
		len = recv(soc, &rbuf[len2], rsize - len2, 0);
		if (len == SOCKET_ERROR) {
			shutdown(soc, 2);
			closesocket(soc);
			return -115;										// Data transmission failure
		}
		if (len > 0) {
			len2 += len;
			// CR-LF reception
			if (rbuf[len2 - 2] == CR && rbuf[len2 - 1] == LF) {
				// In the case of NAK
				if (rbuf[0] == NAK) break;
				// In the case ofACK
				if (rbuf[0] == ACK) break;
				// In the case of STX
				if (rbuf[0] == STX && len2 > 5) {
					if (rbuf[len2 - 5] == ETX) break;
				}
			}
		}

		//Time out if you wait 3 seconds
		if (len < 1) 
			wait_cnt++;
		else 
			wait_cnt = 0;

		if (wait_cnt > 2000) {
			shutdown(soc, 2);					
			closesocket(soc);
			Sleep(30);	
			return(-116);
		}
		Sleep(5);
	}

	shutdown(soc, 2);							// 2:SHUT_RDWR
	closesocket(soc);

	if (!len2) return(-117);

	memset(rcv, 0, rsize);
	memcpy(rcv, rbuf, len2> rsize ? rsize : len2);
	Sleep(10);	
	return(len2);
}
