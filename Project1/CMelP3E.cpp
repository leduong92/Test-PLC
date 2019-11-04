
//#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>
#include "CMelP3E.h"
#include <stdlib.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

CMelsecP3E::CMelsecP3E()
{
	m_fOpen = FALSE;
	m_fBinMode = FALSE;
}

CMelsecP3E::~CMelsecP3E()
{
	WSACleanup();

}

void CMelsecP3E::SetMode(BOOL binaryMode)
{
	if (binaryMode == TRUE)
		m_fBinMode = TRUE;
	else
		m_fBinMode = FALSE;

	return;
}

BOOL CMelsecP3E::Init()
{
	if (m_fOpen)
		return (TRUE);
	
	WSADATA WSAData;

	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
		return -1;

	m_fOpen = TRUE;

	return TRUE;
}
// ============================================================ ============================================================= =======
// int CMelsecP3E :: P3EWrite (char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data)
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
int CMelsecP3E::P3EWrite(char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data)
{
	int ret;

	if (m_fBinMode == TRUE)
		ret = this->P3EWrite(ipaddr, port, devtype, devaddr, counts, data, TRUE);
	else
		ret = this->P3EWrite(ipaddr, port, devtype, devaddr, counts, data, FALSE);

	return ret;
}
// ============================================================ ============================================================= =======
// int CMelsecP3E :: P3EWrite (char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data)
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
// Argument 8: Writing mode (ASCII or binary)
// ============================================================ ============================================================= =======
int CMelsecP3E::P3EWrite(char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data, BOOL binary)
{
	int ret;

	if (m_fBinMode == TRUE) // write bit binary code
		ret = this->P3EWriteB(ipaddr, port, devtype, devaddr, counts, data);
	else //write bit asscii code
		ret = this->P3EWriteA(ipaddr, port, devtype, devaddr, counts, data);

	return ret;
}

int CMelsecP3E::P3EWriteA(char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data)
{
	int ret;
	char send[5120];
	char buf[40 + 1];

	if (m_fOpen == FALSE) return (-1);
	if (counts < 0 || counts > 960) return (-2);
	
	if (strlen(devaddr) != 8) return (-3);

	if (INADDR_NONE == inet_addr(ipaddr)) return (-4);

	if (port < 1024 || port > 65535) return (-5);

	if (devtype == DEV_WORD) {
		if ((short)strlen(data) != counts * 4) return (-6);
	}
	else {
		if ((short)strlen(data) != counts) return (-7);
	}

	memset(send, 0, sizeof(send));
	//Fixed part
	//Sub header "5000" + NW number "00" + PC number "FF" + request destination Unit # "03FF" + request station number "00"
	strcpy(send, "500000FF03FF00");

	//word deivce
	if (devtype == DEV_WORD) {
		//Calculate transmission data length = (Fixed: monitoring timer-reading points) + device points × 4 (bytes)
		sprintf(&send[strlen(send)], "%04X", P3E_FIX_DATALEN + counts * 4);
		//CPU monitoring timer (4) + command (4) + sub command (4)
		strcat(send, "001014010000"); //Word device
	}
	else { //Bit device
		//Calculate transmission data length = (Fixed: monitoring timer-reading points) + device point
		sprintf(&send[strlen(send)], "%04X", P3E_FIX_DATALEN + counts);
		//CPU monitoring timer (4) + command (4) + sub command (4)
		strcat(send, "001014010001"); //Bit device
	}
	//Read address(8)
	strcat(send, devaddr);

	//Transmission data
	sprintf(&send[strlen(send)], "%04X", counts);

	if (devtype == DEV_WORD)
		memcpy(&send[strlen(send)], data, counts * 4);
	else
		memcpy(&send[strlen(send)], data, counts);

	ret = UdpSendA(ipaddr, port, send, strlen(send), buf, sizeof(buf));

	return 0;
}


int CMelsecP3E::UdpSendA(char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize)
{
	SOCKET fSocket;
	WSAEVENT fEvent;
	unsigned long laddr;
	WSABUF wsBuf; //WSABUF structure array
	DWORD dwBufferCount; //Number of WSABUF structures in the array indicated by / lpwsBuf
	DWORD cnt; //Pointer to DWORD variable that receives the number of sent bytes
	DWORD dwFlags; //Bit mask to control transmission method
	struct sockaddr_in addr;
	int ret;
	WSANETWORKEVENTS events;
	int iFromlen; //An integer specifying the length of the sockaddr structure indicated by lpTo
	char rbuf[4096];

	//IP address conversion (to 32 bit)
	if (INADDR_NONE == (laddr = inet_addr(ipaddr))) return (-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fSocket == INVALID_SOCKET) {
		printf("Socket (Error): %ld\n", WSAGetLastError());
		return (-102);
	}
	fEvent = WSACreateEvent();
	if (fEvent == WSA_INVALID_EVENT) {
		printf("\nWSACreateEvent() Error: %d\n", fEvent);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return (-103);
	}

	ret = WSAEventSelect(fSocket, fEvent, FD_READ);
	if (ret == SOCKET_ERROR) {
		printf("\nWSAEventSelect() Error: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-104);
	}

	//Transmission data creation
	wsBuf.len = size;
	wsBuf.buf = data;
	dwBufferCount = 1;
	dwFlags = MSG_DONTROUTE;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = laddr;
	addr.sin_port = htons(port);

	//send
	ret = WSASendTo(fSocket, &wsBuf, dwBufferCount, &cnt, dwFlags, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL);
	if (ret == SOCKET_ERROR) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}

	//Waiting for reception
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
	//Other than received event
	if ((events.lNetworkEvents & FD_READ) == 0) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}

	//Receive
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

	//Check end code (18 bytes from the beginning)
	if (memcmp(rbuf + 18, "0000", 4)) {
		//Output exit code and exit
		memcpy(rcv, rbuf + 18, 4);
		return(1);
	}

	//Since the receive data length (14th byte from the beginning) and the end code (4 bytes) are included, subtract
	ret = this->htoi(rbuf + 14, 4) - 4;

	//Receive data copy
	if (ret>0) {
		memset(rcv, 0, rsize);
		memcpy(rcv, rbuf + 22, ret> rsize ? rsize : ret);
	}

	return 0;
}


int CMelsecP3E::P3EWriteB(char * ipaddr, unsigned short port, int devtype, char * devaddr, short counts, char * data)
{
	unsigned char send[5120];
	int ret, i;
	char buf[40 + 1];
	unsigned short len;
	char txt[30];
	short hex_sign;
	unsigned int top_addres;
	int send_bytes;

	if (m_fOpen == FALSE) return(-1);
	if (counts < 0 || counts > 960) return(-2);

	if (strlen(devaddr) != 8) return(-3);

	if (INADDR_NONE == inet_addr(ipaddr)) return(-4);

	if (port < 1024 || port > 65535) return(-5);

	//Transmission data length
	if (devtype == DEV_WORD) {
		if ((short)strlen(data) != counts * 4) return(-6); //word device
	}
	else
		if ((short)strlen(data) != counts) return(-7); //bit device

	memset(send, 0, sizeof(send));

	send[0] = 0x50; //Subheader
	send[1] = 0x00; //Subheader
	send[2] = 0x00; //Network number
	send[3] = 0xFF; //PC number
	send[4] = 0xFF; //I / O number
	send[5] = 0x03; //Request destination unit
	send[6] = 0x00; //Station number

	//Word device
	if (devtype == DEV_WORD) {
		//Calculate the transmission data length = (fixed: monitoring timer-reading points) + device points × 4 (bytes)
		len = (P3E_FIX_DATALEN + counts * 4) / 2; //send [7] = (P3E_FIX_DATALEN + counts * 4) / 2; // Requested data length
		memcpy(&send[7], &len, 2);
		//CPU monitoring timer (4) + command (4) + sub command (4)
		send[9] = 0x10; //CPU monitoring timer L
		send[10] = 0x00; //CPU monitoring timer H
		send[11] = 0x01; //Command L
		send[12] = 0x14; //Command H
		send[13] = 0x00; //Sub command L
		send[14] = 0x00; //Subcommand H
	}
	else { //Bit device
		//Calculate transmission data length = (Fixed: monitoring timer-reading points) + device points × 4 (bytes)
		if (counts % 2 == 0)
			len = (P3E_FIX_DATALEN + counts) / 2;
		else
			len = (P3E_FIX_DATALEN + counts) / 2 + 1;
		memcpy(&send[7], &len, 2);
		//CPU monitoring timer (4) + command (4) + sub command (4)
		send[9] = 0x10; //CPU monitoring timer L
		send[10] = 0x00; //CPU monitoring timer H
		send[11] = 0x01; //Command L
		send[12] = 0x14; //Command H
		send[13] = 0x01; //Sub command L
		send[14] = 0x00; //Subcommand H
	}

	//Convert to device code
	hex_sign = 0;

	if (!memcmp(devaddr, "D*", 2)) send[18] = 0xa8;	//Data register
	else if (!memcmp(devaddr, "M*", 2)) send[18] = 0x90; //Internal relay		
	else if (!memcmp(devaddr, "SM", 2)) send[18] = 0x91; //Special relay	
	else if (!memcmp(devaddr, "SD", 2)) send[18] = 0xA9; //Special register	
	else if (!memcmp(devaddr, "X*", 2)) {  //Input relay	
		send[18] = 0x9C;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "Y*", 2)) { //Output relay			
		send[18] = 0x9D;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "L*", 2)) send[18] = 0x92; //Latch relay	
	else if (!memcmp(devaddr, "F*", 2)) send[18] = 0x93; //Ananhita		
	else if (!memcmp(devaddr, "V*", 2)) send[18] = 0x94; //Edge relay		
	else if (!memcmp(devaddr, "B*", 2)) {		//Link relay			
		send[18] = 0xA0;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "W*", 2)) {	 //Link register	
		send[18] = 0xB4;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "TS", 2)) send[18] = 0xC1; //Timer contact	
	else if (!memcmp(devaddr, "TC", 2)) send[18] = 0xC0; //Timer coil	
	else if (!memcmp(devaddr, "TN", 2)) send[18] = 0xC2; //Timer Current location		
	else if (!memcmp(devaddr, "SS", 2)) send[18] = 0xC7; //Integration timer contact		
	else if (!memcmp(devaddr, "SC", 2)) send[18] = 0xC6; //Integration timer coil		
	else if (!memcmp(devaddr, "SN", 2)) send[18] = 0xC8; //Integration timer current location		
	else if (!memcmp(devaddr, "CS", 2)) send[18] = 0xC4; //Counter contact	
	else if (!memcmp(devaddr, "CC", 2)) send[18] = 0xC3; //Counter coil		
	else if (!memcmp(devaddr, "CN", 2)) send[18] = 0xC5; //Counter Current location		
	else if (!memcmp(devaddr, "SB", 2)) {	//Special link relay	
		send[18] = 0xA1;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "SW", 2)) {	 //Special link register	
		send[18] = 0xB5;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "S*", 2)) send[18] = 0x98; //Step relay
	else if (!memcmp(devaddr, "DX", 2)) {	//Direct input				
		send[18] = 0xA2;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "DY", 2)) {	//Direct output				
		send[18] = 0xA3;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "Z*", 2)) send[18] = 0xCC; //Index register	
	else if (!memcmp(devaddr, "R*", 2)) send[18] = 0xAF; //Phi register		
	else if (!memcmp(devaddr, "ZR", 2)) {	 //File register			
		send[18] = 0xB0;
		hex_sign = 1;
	}
	else                     return(-100);

	//Read address (8)
	if (hex_sign == 0) {
		memset(txt, 0, sizeof(txt));
		memcpy(txt, devaddr + 2, 6);
		top_addres = atoi(txt);
		memcpy(&send[15], (unsigned int*)&top_addres, 3);
	}
	else {
		memset(txt, 0, sizeof(txt));
		memcpy(txt, devaddr + 6, 2);
		send[15] = (unsigned char)htoi(txt, 2); // Device points L
		memcpy(txt, devaddr + 4, 2);
		send[16] = (unsigned char)htoi(txt, 2); // Device points -
		memcpy(txt, devaddr + 2, 2);
		send[17] = (unsigned char)htoi(txt, 2); // Device score H
	}
	//Reading points
	memcpy(&send[19], (unsigned short*)&counts, 2); //Request data length
	//Transmission data
	//Word device
	if (devtype == DEV_WORD) {
		//1 word = 2 bytes
		for (i = 0; i<counts; i++) {
			//Divide into 2 bytes (HL => LH exchange)
			send[21 + (i * 2)] = (unsigned char)htoi(data + (i * 4) + 2, 2);	// L
			send[21 + (i * 2) + 1] = (unsigned char)htoi(data + (i * 4), 2);	// H
		}
		//Number of transmission bytes (1 word = 2 bytes)
		send_bytes = 21 + counts * 2;
		
	}
	else {
		//Express 2 bits in 1 byte
		for (i = 0; i < counts; i++) {
			if (data[i] == '1') {
				if (i % 2 == 0) send[21 + i / 2] += 0x10;
				else send[21 + i / 2] += 0x1;
			}
		}
		//Number of transmission bytes (2 bits = 1 byte)
		//When 2 bits of data are put into 1 byte data, send [] is advanced by 1
		send_bytes = 21 + counts / 2;
		if (counts % 2) send_bytes++;
	}
	ret = UdpSendB(ipaddr, port, (char*)send, send_bytes, buf, sizeof(buf));

	return(ret);
}

int CMelsecP3E::UdpSendB(char* ipaddr, unsigned short port, char* data, int size, char* rcv, int rsize)
{
	SOCKET fSocket;
	WSAEVENT fEvent;
	//DWORD		cEvents;		
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
	int i;
	unsigned short dcnt, *dat;

	if (INADDR_NONE == (laddr = inet_addr(ipaddr))) return(-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fSocket == INVALID_SOCKET) {
		printf("socket()Err");
		return(-102);
	}
	fEvent = WSACreateEvent();
	if (fEvent == WSA_INVALID_EVENT) {
		printf("\nWSACreateEvent()Err: %d\n", fEvent);
		closesocket(fSocket);
		return(-103);
	}

	ret = WSAEventSelect(fSocket, fEvent, FD_READ);
	if (ret == SOCKET_ERROR) {
		printf("\nWSAEventSelect()Err: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-104);
	}

	//Transmission data creation
	wsBuf.len = size;
	wsBuf.buf = data;
	dwBufferCount = 1;
	dwFlags = MSG_DONTROUTE;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = laddr;
	addr.sin_port = htons(port);

	ret = WSASendTo(fSocket, &wsBuf, dwBufferCount, &cnt, dwFlags,
		(LPSOCKADDR)&addr, sizeof(addr), NULL, NULL);
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

	//Check exit code
	if (rbuf[9] != 0x00 || rbuf[10] != 0x00) {
		printf("\nrcv=%d%d", rbuf[9], rbuf[10]);
		//Output exit code and exit
		sprintf(rcv, "%02d%02d", rbuf[9], rbuf[10]);
		return(1);
	}
	//Since the receive data length (the first and seventh bytes from the beginning) and the end code (2 bytes) are included, it is subtracted
	//Divide by 2 because 4 characters can be represented by 2 bytes
	memcpy(&dcnt, &rbuf[7], 2);
	dcnt = (dcnt - 2) / 2;

	if (dcnt>0)
	{
		memset(rcv, 0, rsize);
		for (i = 0; i<dcnt; i++)
		{
			//Data position (from the 11th received data)
			dat = (unsigned short*)(rbuf + 11 + i * 2);
			sprintf(&rcv[i * 4], "%04X", *dat);
		}
	}

	return(0);
}
// ============================================================ ============================================================= =======
// int CMelsecP3E :: P3ERead (char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts)
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
int CMelsecP3E::P3ERead(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts)
{
	int ret;

	if (m_fBinMode == TRUE)
		ret = this->P3ERead(ipaddr, port, buf, bufsize, devtype, devaddr, counts, TRUE);
	else
		ret = this->P3ERead(ipaddr, port, buf, bufsize, devtype, devaddr, counts, FALSE);

	return ret;
}

int CMelsecP3E::P3ERead(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts, BOOL binary)
{
	int ret;

	if (binary == TRUE)
		ret = this->P3EReadB(ipaddr, port, buf, bufsize, devtype, devaddr, counts);
	else
		ret = this->P3EReadA(ipaddr, port, buf, bufsize, devtype, devaddr, counts);

	return ret;
}

int CMelsecP3E::P3EReadA(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts)
{
	char send[5120];
	int ret;

	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 960) return(-2);

	if (strlen(devaddr) != 8) return(-3);

	if (INADDR_NONE == inet_addr(ipaddr)) return(-4);

	if (port <1024 || port >65535) return(-5);

	memset(send, 0, sizeof(send));

	strcpy(send, "500000FF03FF00");

	sprintf(&send[strlen(send)], "%04X", P3E_FIX_DATALEN);


	if (devtype == DEV_WORD)
		strcat(send, "001004010000");	
	else
		strcat(send, "001004010001");	

										
	strcat(send, devaddr);

	sprintf(&send[strlen(send)], "%04X", counts);

	ret = UdpSendA(ipaddr, port, send, strlen(send), buf, bufsize);

	return(ret);
}

int CMelsecP3E::P3EReadB(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts)
{

	unsigned char send[120];
	int ret;
	unsigned short len;
	char txt[30];
	short hex_sign;	// Sign indicating whether the read address specification method is hexadecimal: 0: decimal 1: hexadecimal
	unsigned int top_addres;

	if (m_fOpen == FALSE) return(-1);
	if (counts <0 || counts > 960) return(-2);

	// Address (device code (2) + top device (6))
	if (strlen(devaddr) != 8) return(-3);

	if (INADDR_NONE == inet_addr(ipaddr)) return(-4);

	if (port <1024 || port >65535) return(-5);


	memset(send, 0, sizeof(send));
	// Subheader "5000" + NW number "00" + PC number "FF" + request destination Unit # "03FF" + request station number "00"
	send[0] = 0x50;// Subheader
	send[1] = 0x00;// Subheader
	send[2] = 0x00;// Network number
	send[3] = 0xFF;// PC number
	send[4] = 0xFF;// Request destination unit
	send[5] = 0x03;// I / O number
	send[6] = 0x00;// Station number

	//Calculate transmission data length = read, so fixed length (monitoring timer-read points)
	len = P3E_FIX_DATALEN / 2;
	memcpy(&send[7], &len, 2);// Request data length 0x0c

	//CPU monitoring timer (4) + command (4) + sub command (4)
	if (devtype == DEV_WORD) //Word device
	{
		//001004010000
		send[9] = 0x10;// CPU monitoring timer L
		send[10] = 0x00;// CPU monitoring timer H
		send[11] = 0x01;// Command L
		send[12] = 0x04;// Command H
		send[13] = 0x00;// Sub command L
		send[14] = 0x00;// Sub command H
	}
	else
	{
		send[9] = 0x10;// CPU monitoring timer L
		send[10] = 0x00;// CPU monitoring timer H
		send[11] = 0x01;// Command L
		send[12] = 0x04;// Command H
		send[13] = 0x01;// Sub command L
		send[14] = 0x00;// Sub command H
	}

	// Convert to device code
	hex_sign = 0;

	if (!memcmp(devaddr, "D*", 2)) send[18] = 0xa8;			// Data register
	else if (!memcmp(devaddr, "M*", 2)) send[18] = 0x90;		
	else if (!memcmp(devaddr, "SM", 2)) send[18] = 0x91;		
	else if (!memcmp(devaddr, "SD", 2)) send[18] = 0xA9;		
	else if (!memcmp(devaddr, "X*", 2)) {					
		send[18] = 0x9C;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "Y*", 2)) {					
		send[18] = 0x9D;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "L*", 2)) send[18] = 0x92;		
	else if (!memcmp(devaddr, "F*", 2)) send[18] = 0x93;		
	else if (!memcmp(devaddr, "V*", 2)) send[18] = 0x94;		
	else if (!memcmp(devaddr, "B*", 2)) {					
		send[18] = 0xA0;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "W*", 2)) {				
		send[18] = 0xB4;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "TS", 2)) send[18] = 0xC1;		
	else if (!memcmp(devaddr, "TC", 2)) send[18] = 0xC0;		
	else if (!memcmp(devaddr, "TN", 2)) send[18] = 0xC2;		
	else if (!memcmp(devaddr, "SS", 2)) send[18] = 0xC7;		
	else if (!memcmp(devaddr, "SC", 2)) send[18] = 0xC6;		
	else if (!memcmp(devaddr, "SN", 2)) send[18] = 0xC8;		
	else if (!memcmp(devaddr, "CS", 2)) send[18] = 0xC4;	
	else if (!memcmp(devaddr, "CC", 2)) send[18] = 0xC3;		
	else if (!memcmp(devaddr, "CN", 2)) send[18] = 0xC5;		
	else if (!memcmp(devaddr, "SB", 2)) {					
		send[18] = 0xA1;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "SW", 2)) {					
		send[18] = 0xB5;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "S*", 2)) send[18] = 0x98;		
	else if (!memcmp(devaddr, "DX", 2)) {					
		send[18] = 0xA2;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "DY", 2)) {					
		send[18] = 0xA3;
		hex_sign = 1;
	}
	else if (!memcmp(devaddr, "Z*", 2)) send[18] = 0xCC;		
	else if (!memcmp(devaddr, "R*", 2)) send[18] = 0xAF;		
	else if (!memcmp(devaddr, "ZR", 2)) {					
		send[18] = 0xB0;
		hex_sign = 1;
	}
	else                     return(-100);

	// Read address (8)
	if (hex_sign == 0) {
		memset(txt, 0, sizeof(txt));
		memcpy(txt, devaddr + 2, 6);
		top_addres = atoi(txt);
		memcpy(&send[15], (unsigned int*)&top_addres, 3);
	}
	else {
		memset(txt, 0, sizeof(txt));
		memcpy(txt, devaddr + 6, 2);
		send[15] = (unsigned char)htoi(txt, 2);// Device score L
		memcpy(txt, devaddr + 4, 2);
		send[16] = (unsigned char)htoi(txt, 2);// Device score -
		memcpy(txt, devaddr + 2, 2);
		send[17] = (unsigned char)htoi(txt, 2);//Device score- H	
	}


	// Reading points
	memcpy(&send[19], (unsigned short*)&counts, 2);
	
	if (devtype == DEV_WORD)
		ret = UdpSendB(ipaddr, port, (char*)send, 21, buf, bufsize);
	else
		ret = UdpSendBit(ipaddr, port, (char*)send, 21, buf, bufsize, counts);

	return(ret);
}

// ============================================================ ============================================================= =======
// int CMelsecP3E :: UdpSend (char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize)
// type: private
// Function: UDP transmission / reception (BIT device read, dedicated to BINARY mode)
// Argument 1: Target IP address
// Argument 2: Target UDP port number
// Argument 3: Transmission data
// Argument 4: Transmission data size
// Argument 5: Receive buffer
// Argument 6: Receive buffer size
// Argument 7: Received data points
// Return value: 0 = OK, less than 0 = network error, 1 = received data error
// ============================================================ ============================================================= =======
int CMelsecP3E::UdpSendBit(char * ipaddr, unsigned short port, char * data, int size, char * rcv, int rsize, short counts)
{
	SOCKET fSocket;
	WSAEVENT fEvent;
	unsigned long laddr;
	WSABUF		wsBuf;	
	DWORD		dwBufferCount;	// number of WSABUF structures in the array indicated by lpwsBuf
	DWORD		cnt; // A pointer to a DWORD variable that receives the number of bytes to send
	DWORD		dwFlags;		// Bit mask to control transmission method
	struct sockaddr_in	addr;
	int ret;
	WSANETWORKEVENTS events;
	int			iFromlen;// An integer specifying the length of the sockaddr structure pointed to by lpTo
	char rbuf[4096];
	int i;
	unsigned short dcnt;

	// IP address conversion (to 32bit)
	if (INADDR_NONE == (laddr = inet_addr(ipaddr))) return(-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fSocket == INVALID_SOCKET) {
		printf("socket()Err");
		return(-102);
	}
	fEvent = WSACreateEvent();
	if (fEvent == WSA_INVALID_EVENT) {
		printf("\nWSACreateEvent()Err: %d\n", fEvent);
		closesocket(fSocket);
		return(-103);
	}

	ret = WSAEventSelect(fSocket, fEvent, FD_READ);
	if (ret == SOCKET_ERROR) {
		printf("\nWSAEventSelect()Err: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-104);
	}

	// Transmission data creation
	wsBuf.len = size;
	wsBuf.buf = data;
	dwBufferCount = 1;
	dwFlags = MSG_DONTROUTE;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = laddr;
	addr.sin_port = htons(port);

	ret = WSASendTo(fSocket, &wsBuf, dwBufferCount, &cnt, dwFlags,
		(LPSOCKADDR)&addr, sizeof(addr), NULL, NULL);
	if (ret == SOCKET_ERROR) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}

	// Waiting for reception
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

	// Other than received event
	if ((events.lNetworkEvents & FD_READ) == 0) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}

	// To receive
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

	// check exit code
	if (rbuf[9] != 0x00 || rbuf[10] != 0x00) {
		printf("\nrcv=%d%d", rbuf[9], rbuf[10]);
		// Exit code and exit
		sprintf(rcv, "%02d%02d", rbuf[9], rbuf[10]);
		return(1);
	}

	// Since the receive data length (the first and seventh bytes from the beginning) and the end code (2 bytes) are included, it is subtracted
	memcpy(&dcnt, &rbuf[7], 2);
	dcnt = dcnt - 2;

	// Receive data copy
	if (dcnt>0)
	{
		memset(rcv, 0, rsize);
		for (i = 0; i<dcnt; i++)
		{
			// Data position (from the 11th received data)
			sprintf(&rcv[i * 2], "%02X", rbuf[11 + i]);
		}
	}

	// If the number of data read points is odd, the last digit is a dummy data and it is erased
	if (counts % 2 == 1)	rcv[counts] = 0;
	return(0);
}

//  ================================================================================================================================
// unsigned char CMelsecP3E :: htoi (char * hexstr, short len)
// Function: Convert hexadecimal (only 2 characters string) to integer value (unsigned char type)
// Argument: Conversion source data storage pointer
// Return value: Converted data
// ================================================================================================================================
unsigned short CMelsecP3E::htoi(char *hexstr, short len)
{
	unsigned short ret = 0;
	short i;

	if (len > 4) return 0;
	for (i = 0; i < len; i++) {
		ret *= 16;
		if (hexstr[i] < 'A') 
			ret += hexstr[i] - '0';
		else
			if (hexstr[i] < 'a') 
				ret += hexstr[i] - 'A' + 10;
			else
				ret += hexstr[i] - 'a' + 10;
	}
	return(ret);
}

int CMelsecP3E::foo(char * input, char * output)
{
	SOCKET fSocket;
	WSAEVENT fEvent;
	unsigned long laddr;
	WSABUF wsBuf; //WSABUF structure array
	DWORD dwBufferCount; //Number of WSABUF structures in the array indicated by / lpwsBuf
	DWORD cnt; //Pointer to DWORD variable that receives the number of sent bytes
	DWORD dwFlags; //Bit mask to control transmission method
	struct sockaddr_in addr;
	int ret;
	WSANETWORKEVENTS events;
	int iFromlen; //An integer specifying the length of the sockaddr structure indicated by lpTo
	char rbuf[4096];

	//IP address conversion (to 32 bit)
	if (INADDR_NONE == (laddr = inet_addr("10.203.83.61"))) return (-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fSocket == INVALID_SOCKET) {
		printf("Socket (Error): %ld\n", WSAGetLastError());
		return (-102);
	}
	fEvent = WSACreateEvent();
	if (fEvent == WSA_INVALID_EVENT) {
		printf("\nWSACreateEvent() Error: %d\n", fEvent);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return (-103);
	}

	ret = WSAEventSelect(fSocket, fEvent, FD_READ);
	if (ret == SOCKET_ERROR) {
		printf("\nWSAEventSelect() Error: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-104);
	}

	//Transmission data creation
	wsBuf.len = 42;
	wsBuf.buf = input;
	dwBufferCount = 1;
	dwFlags = MSG_DONTROUTE;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = laddr;
	addr.sin_port = htons(25884);

	//send
	ret = WSASendTo(fSocket, &wsBuf, dwBufferCount, &cnt, dwFlags, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL);
	if (ret == SOCKET_ERROR) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}

	//Waiting for reception
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
	//Other than received event
	if ((events.lNetworkEvents & FD_READ) == 0) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}

	//Receive
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

	//Check end code (18 bytes from the beginning)
	if (memcmp(rbuf + 18, "0000", 4)) {
		//Output exit code and exit
		memcpy(output, rbuf + 18, 4);
		return(1);
	}

	//Since the receive data length (14th byte from the beginning) and the end code (4 bytes) are included, subtract
	ret = this->htoi(rbuf + 14, 4) - 4;

	//Receive data copy
	if (ret>0) {
		memset(output, 0, strlen(rbuf));
		memcpy(output, rbuf + 22, ret> strlen(rbuf) ? strlen(rbuf) : ret);
	}

	return 0;
}

int CMelsecP3E::P3EReadAscii(char * ipaddr, unsigned short port, char * buf, int bufsize, int devtype, char * devaddr, short counts, int modeBinaryOrAscii)
{
	char send[5120];
	int ret;
	SOCKET fSocket;
	WSAEVENT fEvent;
	unsigned long laddr;
	WSABUF wsBuf; //WSABUF structure array
	DWORD dwBufferCount; //Number of WSABUF structures in the array indicated by / lpwsBuf
	DWORD cnt; //Pointer to DWORD variable that receives the number of sent bytes
	DWORD dwFlags; //Bit mask to control transmission method
	struct sockaddr_in addr;
	WSANETWORKEVENTS events;
	int iFromlen; //An integer specifying the length of the sockaddr structure indicated by lpTo
	char rbuf[4096];

	//if (m_fOpen == FALSE) return(-100);

	if (modeBinaryOrAscii == TRUE) return (-1000);

	if (counts <0 || counts > 960) return(-2);

	if (strlen(devaddr) != 8) return(-3);

	if (INADDR_NONE == inet_addr(ipaddr)) return(-4);

	if (port <1024 || port >65535) return(-5);

	memset(send, 0, sizeof(send));

	strcpy(send, "500000FF03FF00");

	sprintf(&send[strlen(send)], "%04X", P3E_FIX_DATALEN);

	if (devtype == DEV_WORD)
		strcat(send, "001004010000");
	else
		strcat(send, "001004010001");

	strcat(send, devaddr);

	sprintf(&send[strlen(send)], "%04X", counts);

	//IP address conversion (to 32 bit)
	if (INADDR_NONE == (laddr = inet_addr(ipaddr))) return (-101);

	fSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fSocket == INVALID_SOCKET) {
		printf("Socket (Error): %ld\n", WSAGetLastError());
		return (-102);
	}
	fEvent = WSACreateEvent();
	if (fEvent == WSA_INVALID_EVENT) {
		printf("\nWSACreateEvent() Error: %d\n", fEvent);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return (-103);
	}
	ret = WSAEventSelect(fSocket, fEvent, FD_READ);
	if (ret == SOCKET_ERROR) {
		printf("\nWSAEventSelect() Error: %d\n", ret);
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-104);
	}
	//Transmission data creation
	wsBuf.len = strlen(send);
	wsBuf.buf = send;
	dwBufferCount = 1;
	dwFlags = MSG_DONTROUTE;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = laddr;
	addr.sin_port = htons(port);

	//send
	ret = WSASendTo(fSocket, &wsBuf, dwBufferCount, &cnt, dwFlags, (LPSOCKADDR)&addr, sizeof(addr), NULL, NULL);
	if (ret == SOCKET_ERROR) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-105);
	}

	//Waiting for reception
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
	//Other than received event
	if ((events.lNetworkEvents & FD_READ) == 0) {
		WSACloseEvent(fEvent);
		closesocket(fSocket);
		return(-108);
	}
	//Receive
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

	//Check end code (18 bytes from the beginning)
	if (memcmp(rbuf + 18, "0000", 4)) {
		//Output exit code and exit
		memcpy(buf, rbuf + 18, 4);
		return(1);
	}
	//Since the receive data length (14th byte from the beginning) and the end code (4 bytes) are included, subtract
	ret = this->htoi(rbuf + 14, 4) - 4;

	//Receive data copy
	if (ret>0) {
		memset(buf, 0, bufsize);
		memcpy(buf, rbuf + 22, ret > bufsize ? bufsize : ret);
	}
	return 0;
}

