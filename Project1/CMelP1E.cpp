#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include "CMelP1E.h"
#include <ws2tcpip.h>

CMelsecP1E::CMelsecP1E()
{
	m_fOpen = FALSE;
	m_tcp = FALSE;	
}

CMelsecP1E::~CMelsecP1E()
{
	WSACleanup();
}

BOOL CMelsecP1E::Init(BOOL tcp)
{
	if (m_fOpen)
		return TRUE;

	WSADATA WSAData;

	if (WSAStartup(MAKEWORD(2, 2), &WSAData)) return(-1);

	m_fOpen = TRUE;

	if (tcp) 
		m_tcp = TRUE;
	else 
		m_tcp = FALSE;

	return(TRUE);
}
// ============================================================ ============================================================= =======
// int CMelsecP1E :: P3EWrite (char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data)
// type: public
// Function: Block transmission of device data in protocol 3E format
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: Receive buffer
// Argument 4: Receive buffer size
// Argument 5: type of device (0 = word device, 1 = bit device)
// Argument 5: Write start address (8 digits)
// Argument 6: Number of transmission blocks (max 256)
// Argument 7: write data
// ============================================================ ============================================================= =======
int CMelsecP1E::P1EWrite(char * ipaddr, unsigned short port, int devtype, char * devname, int devaddr, short counts, char * data)
{
	//Send buffer
	char send[4096];
	int ret;
	char buf[40 + 1];
	unsigned char buf2[30];
	unsigned long ldat;

	// Unopened error
	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 256) return(-2);

	if (INADDR_NONE == inet_addr(ipaddr)) return(-4);

	if (port < 1024 || port > 65535) return(-5);

	memset(send, 0, sizeof(send));

	// subheader (command number) + PC number (FF fixed)
	if (devtype == DEV_WORD) 
		strcpy(send, "03FF");
	else 
		strcpy(send, "02FF");

	// Monitoring timer (use 000A)
	memcpy(send + 4, "000A", 4);

	// Start device name
	memset(buf, 0x20, 2);
	memcpy(buf, devname, strlen(devname) > 2 ? 2 : strlen(devname));
	sprintf(&send[strlen(send)], "%02X%02X", buf[0], buf[1]);

	// Start device Convert to long type and convert 1 byte into string. Pay attention to the order
	ldat = (unsigned long)devaddr;	// Storage order of ldat → LL LH HL HH (L: Low, H: High)

	memcpy(buf2, &ldat, 4);
	sprintf(&send[strlen(send)], "%02X%02X%02X%02X", buf2[3], buf2[2], buf2[1], buf2[0]);

	// Number of data points
	sprintf(&send[strlen(send)], "%02X", counts);
	// terminator
	memcpy(&send[strlen(send)], "00", 2);

	//Transmission data
	if (devtype == DEV_WORD) {
		memcpy(&send[strlen(send)], data, counts * 4);
	}
	else {
		memcpy(&send[strlen(send)], data, counts);
		// Add a dummy if it is an odd number
		if (counts % 2)
			memcpy(&send[strlen(send)], "0", 1);
	}

	// Send
	// TCP communication
	if (m_tcp)
		ret = TcpSend(ipaddr, port, send, strlen(send), buf, sizeof(buf));
	else 
		ret = UdpSend(ipaddr, port, send, strlen(send), buf, sizeof(buf));

	return(ret);
}
// ============================================================ ============================================================= =======
// int CMelsecP1E :: P3ERead (char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts)
// type: public
// Function: Receive block of device data in protocol 3 E format
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: Receive buffer
// Argument 4: Receive buffer size
// Argument 5: type of device (0 = word device, 1 = bit device)
// Argument 5: Read start address (8 digits)
// Argument 6: Number of received blocks (max 256)
// ============================================================ ============================================================= =======
int CMelsecP1E::P1ERead(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devname, int devaddr, short counts)
{
	char send[4096];
	int ret;
	char buf1[30];
	unsigned char buf2[30];
	unsigned long ldat;

	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 256) return(-2);

	if (INADDR_NONE == inet_addr(ipaddr)) return(-4);

	if (port <1024 || port >65535) return(-5);

	memset(send, 0, sizeof(send));

	if (devtype == DEV_WORD) 
		strcpy(send, "01FF");
	else 
		strcpy(send, "00FF");

	memcpy(send + 4, "000A", 4);

	memset(buf1, 0x20, 2);
	memcpy(buf1, devname, strlen(devname) > 2 ? 2 : strlen(devname));
	sprintf(&send[strlen(send)], "%02X%02X", buf1[0], buf1[1]);

	ldat = (unsigned long)devaddr;
	memcpy(buf2, &ldat, 4);
	sprintf(&send[strlen(send)], "%02X%02X%02X%02X", buf2[3], buf2[2], buf2[1], buf2[0]);

	sprintf(&send[strlen(send)], "%02X", counts);

	memcpy(&send[strlen(send)], "00", 2);

	if (m_tcp)
		ret = TcpSend(ipaddr, port, send, strlen(send), buf, bufsize);
	else 
		ret = UdpSend(ipaddr, port, send, strlen(send), buf, bufsize);

	return(ret);
}

// ============================================================ ============================================================= =======
// int CMelsecP1E :: Str2Mel (char * buf, unsigned int bufsize, char * str)
// Function: Converts a character string to Melsec transmission format
// ex) "A1234"-> "31413433"
// Argument 1: Output buffer
// Argument 2: Buffer size (twice the conversion source data length + 1 is necessary)
// Argument 3: conversion source string data (NULL-terminated)
// Return value: 0 = OK
// ============================================================ ============================================================= =======
int CMelsecP1E::Str2Mel(char * buf, unsigned int bufsize, char * str)
{
	unsigned int i, len;
	char txt[10];

	len = strlen(str);
	if (len * 2 >= bufsize) return(-1);

	memset(buf, 0, bufsize);

	for (i = 0; i<len; i++) {
		// Swap up and down
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

// ============================================================ ============================================================= =======
// unsigned char CMelsecP1E :: htoi (char * hexstr, short len)
// Function: Convert hexadecimal (only 2 characters string) to integer value (unsigned char type)
// Argument: Conversion source data storage pointer
// Return value: Converted data
// ============================================================ ============================================================= =======
int CMelsecP1E::Mel2Str(char * buf, unsigned int bufsize, char * melstr)
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

BOOL CMelsecP1E::ULChg(char * buf)
{
	return 0;
}
// ============================================================ ============================================================= =======
// 2012.12.21 coba addition
// int CMelsecP1E :: TcpSend (char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize)
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
int CMelsecP1E::TcpSend(char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize)
{
	unsigned long laddr;
	SOCKET soc;
	struct sockaddr_in rmthost;
	int ret;
	unsigned long arg;
	int len, len2, dlen;
	int wait_cnt, chk;
	fd_set r_socket;
	struct timeval time;
	char rbuf[4096];

	if (INADDR_NONE == (laddr = inet_addr(ipaddr))) return (-101);

	if (size < 1) return (-103);

	soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (soc == INVALID_SOCKET) return (-104);

	memset(&rmthost, 0, sizeof(rmthost));
	rmthost.sin_family = AF_INET;
	rmthost.sin_port = htons((unsigned short)port);
	rmthost.sin_addr.s_addr = laddr;

	ret = connect(soc, (struct sockaddr*)&rmthost, sizeof(struct sockaddr));
	if (ret == SOCKET_ERROR) {
		closesocket(soc);
		Sleep(30);
		return (-105);
	}

	arg = 0;
	if ((ioctlsocket(soc, FIONBIO, &arg)) != 0) {
		closesocket(soc);
		Sleep(30);
		return (-106);
	}
	FD_ZERO(&r_socket);
	FD_SET(soc, &r_socket);
	time.tv_sec = 2000;	
	time.tv_usec = 0;

	ret = select(1, NULL, &r_socket, NULL, &time);
	if (ret == SOCKET_ERROR) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -107;										
	}
	else if (ret == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -108;										
	}
	else if (FD_ISSET(soc, &r_socket) == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -109;										
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
			return -110;										
		}
		
		len2 += ret; //// Number of sent bytes
		if (len != len2) {
			if (++wait_cnt > 4000) {
				shutdown(soc, 2);
				closesocket(soc);
				Sleep(30);	
				return -111;										
			}
			Sleep(5);
		}
	
		else wait_cnt = 0;
	} while (len != len2);	//Until it can be sent			

	FD_ZERO(&r_socket);
	FD_SET(soc, &r_socket);
	time.tv_sec = 2000;  
	time.tv_usec = 0;

	ret = select(1, &r_socket, NULL, NULL, &time);
	if (ret == SOCKET_ERROR) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -112;								
	}
	else if (ret == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);	
		return -113;									
	}
	else if (FD_ISSET(soc, &r_socket) == 0) {
		shutdown(soc, 2);
		closesocket(soc);
		Sleep(30);
		return -114;										
	}
	len = 0, len2 = 0, wait_cnt = 0, dlen = 0, chk = 0;
	memset(rbuf, 0, sizeof(rbuf));
	while (1) {
		len = recv(soc, &rbuf[len2], rsize - len2, 0);
		if (len == SOCKET_ERROR) {
			shutdown(soc, 2);
			closesocket(soc);
			return -115;									
		}
		if (len > 0) {
			len2 += len;
			break;
		}

		// Wait 3 seconds and time out
		if (len < 1) wait_cnt++;
		else wait_cnt = 0;
		if (wait_cnt > 2000) {
			shutdown(soc, 2);					
			closesocket(soc);
			Sleep(30);	
			return(-116);
		}
		Sleep(5);
	}
	shutdown(soc, 2);						
	closesocket(soc);

	// Check end code (2 bytes from the beginning)
	memset(rcv, 0, rsize);
	if (memcmp(rbuf + 2, "00", 2)) {
		memcpy(rcv, rbuf + 2, 4);
		Sleep(30);	
		return(1);
	}

	ret = strlen(rbuf + 4);

	if (ret > 0) {
		memcpy(rcv, rbuf + 4, ret> rsize ? rsize : ret);
	}

	Sleep(30);

	return 0;
}

int CMelsecP1E::UdpSend(char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize)
{
	SOCKET fSocket;
	WSAEVENT fEvent;
	unsigned long laddr;
	WSABUF wsBuf;
	DWORD dwBufferCount;
	DWORD cnt;
	DWORD dwFlags;
	struct sockaddr_in addr;
	int ret;
	WSANETWORKEVENTS events;
	int iFromlen;
	char rbuf[4096];

	if (INADDR_NONE == (laddr = inet_addr(ipaddr))) return (-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fSocket == INVALID_SOCKET) {
		printf("Socket() Error");
		return (-102);
	}
	fEvent = WSACreateEvent();
	if (fEvent == WSA_INVALID_EVENT) {
		printf("\nWSACreateEvent() Err: %d\n", fEvent);
		closesocket(fSocket);
		return(-103);
	}
	ret = WSAEventSelect(fSocket, fEvent, FD_READ);
	if (ret == SOCKET_ERROR) {
		printf("\nWSAEventSelect() Err: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return (-104);
	}

	wsBuf.len = size;
	wsBuf.buf = data;
	dwBufferCount = 1;
	dwFlags = MSG_DONTROUTE;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = laddr;
	addr.sin_port = htons(port);

	ret = WSASendTo(fSocket, &wsBuf, dwBufferCount, &cnt, dwFlags, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL);
	if (ret == SOCKET_ERROR) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}

	ret = WSAWaitForMultipleEvents(1, &fEvent, TRUE, 10000, TRUE);	
	if (ret == WSA_WAIT_TIMEOUT) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-106);
	}

	ret = WSAEnumNetworkEvents(fSocket, fEvent, &events);
	if (ret == SOCKET_ERROR) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-107);
	}

	if ((events.lNetworkEvents & FD_READ) == 0) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}

	memset(rbuf, 0, sizeof(rbuf));
	wsBuf.len = sizeof(rbuf);
	wsBuf.buf = rbuf;
	dwBufferCount = 1;
	dwFlags = 0;
	memset(&addr, 0, sizeof(addr));
	iFromlen = sizeof(addr);

	ret = WSARecvFrom(fSocket, &wsBuf, dwBufferCount, &cnt, &dwFlags,
		(LPSOCKADDR)&addr, &iFromlen, NULL, NULL);

	WSACloseEvent(fEvent);
	closesocket(fSocket);

	if (memcmp(rbuf + 2, "00", 2)) {
		memcpy(rcv, rbuf + 2, 4);
		return(1);
	}

	ret = strlen(rbuf + 4);

	if (ret>0) {
		memset(rcv, 0, rsize);
		memcpy(rcv, rbuf + 4, ret> rsize ? rsize : ret);
	}
	return 0;
}
// ============================================================ ============================================================= =======
// unsigned char CMelsecP1E :: htoi (char * hexstr, short len)
// Function: Convert hexadecimal (only 2 characters string) to integer value (unsigned char type)
// Argument: Conversion source data storage pointer
// Return value: Converted data
// ============================================================ ============================================================= =======
unsigned char CMelsecP1E::htoi(char * hexstr, short len)
{
	return 0;
}
