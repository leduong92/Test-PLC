#include "stdafx.h"
#include "RS232.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <stdint.h>
#include <conio.h>
#define BUF_SIZE (61)
#define READ_TIMEOUT      500      // milliseconds

TRS232C::TRS232C()
{
	m_hComm = NULL;
	m_fOpen = FALSE;
}

TRS232C::~TRS232C()
{
	if (m_hComm != NULL)
		CloseHandle(m_hComm);
	m_fOpen = FALSE;
}
BOOL TRS232C::CommOpen(void)
{
	return (CommOpen(1, NULL));
}

BOOL TRS232C::CommOpen(long comNo, PORTSETTING* ps)
{
	if (m_fOpen)
		return FALSE;

	if (comNo < 1)
		return FALSE;

	char theComm[20];
	if (comNo >= 10)
	{
		wsprintf(theComm, "\\\\.\\COM%ld", comNo);
	} else {
		wsprintf(theComm, "COM%ld", comNo);
	}

	DWORD fdwDesiredAccess, fdwShareMode, fSendBuffer = 4096, fReceiveBuffer = 4096;
	LPSECURITY_ATTRIBUTES lpSecurityAttributes;
	DWORD fdwCreationDisposition, fdwFlagsAndAttributes;
	HANDLE hTemplateFile;
	COMMTIMEOUTS CommTimeOuts;

	fdwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	fdwShareMode = 0;
	lpSecurityAttributes = NULL;
	fdwCreationDisposition = OPEN_EXISTING;
	fdwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	hTemplateFile = NULL;

	m_hComm = CreateFile(theComm, fdwDesiredAccess, fdwShareMode, lpSecurityAttributes, fdwCreationDisposition, fdwFlagsAndAttributes, hTemplateFile);

	if (m_hComm == INVALID_HANDLE_VALUE) {
		system_error("opening file");
		m_hComm = NULL;
		return (FALSE);
	}

	SetCommMask(m_hComm, 0); //If the function succeeds, the return value is nonzero. A value of zero disables all events

	SetupComm(
		m_hComm,
		fSendBuffer,
		fReceiveBuffer
	);
	
	PurgeComm(m_hComm, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR);

	CommTimeOuts.ReadIntervalTimeout = MAXDWORD;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 5;
	CommTimeOuts.WriteTotalTimeoutConstant = 1500;

	SetCommTimeouts(m_hComm, &CommTimeOuts);  //If the function succeeds, the return value is nonzero.

	DCB dcb; //Device Control Block

	GetCommState(m_hComm, &dcb);  //Get Current DCB, //If the function succeeds, the return value is nonzero.

	dcb.BaudRate = ps->BauRate; //Baudrate at which running 
	dcb.fBinary = ps->fBinary; // Binary Mode (skip EOF check) true or false
	dcb.fOutxCtsFlow = ps->fOutxCtsFlow;
	/*==================================fOutxCtsFlow=====================================
	//	If this member is TRUE, the CTS (clear-to-send) signal is monitored for output flow control. If this member is TRUE and CTS is turned off, output is suspended until CTS is sent again.
	//==================================fOutxCtsFlow=====================================*/
	dcb.fOutxDsrFlow = ps->fOutxDsrFlow; 
	/*==================================fOutxDsrFlow=====================================
	//	If this member is TRUE, the DSR (data-set-ready) signal is monitored for output flow control. If this member is TRUE and DSR is turned off, output is suspended until DSR is sent again.
	//==================================fOutxDsrFlow=====================================*/
	dcb.fDtrControl = ps->fDtrControl; 
	/*==================================fDtrControl=====================================
	//	DTR_CONTROL_DISABLE 0x00: Disables the DTR line when the device is opened and leaves it disabled.
	//	DTR_CONTROL_ENABLE 0x01: Enables the DTR line when the device is opened and leaves it on.
	//	DTR_CONTROL_HANDSHAKE 0x02: Enables DTR handshaking. If handshaking is enabled, it is an error for the application to adjust the line by using the EscapeCommFunction function.
	//==================================fDtrControl=====================================*/
	dcb.fRtsControl = ps->fRtsControl; 
	/*==================================fRtsControl=====================================
	//	RTS_CONTROL_DISABLE 0x00: Disables the RTS line when the device is opened and leaves it disabled.
	//	RTS_CONTROL_ENABLE 0x01: Enables the RTS line when the device is opened and leaves it on.
	//	RTS_CONTROL_HANDSHAKE 0x02: Enables RTS handshaking. The driver raises the RTS line when the "type-ahead" (input) buffer is less than one-half full and lowers the RTS line when the buffer is more than three-quarters full. If handshaking is enabled, it is an error for the application to adjust the line by using the EscapeCommFunction function.
	//	RTS_CONTROL_TOGGLE 0x03: Specifies that the RTS line will be high if bytes are available for transmission. After all buffered bytes have been sent, the RTS line will be low.
	//==================================fRtsControl=====================================*/

	dcb.fTXContinueOnXoff = FALSE;
	/*==================================fTXContinueOnXoff===================================== 
	//	If this member is TRUE, transmission continues after the input buffer has come within XoffLim bytes of being full and the driver has transmitted 
	//	the XoffChar character to stop receiving bytes. If this member is FALSE, transmission does not continue until the input buffer is within XonLim bytes 
	//	of being empty and the driver has transmitted the XonChar character to resume reception.
	//==================================fTXContinueOnXoff=====================================*/
	dcb.fInX = dcb.fOutX = FALSE;
	/*==================================fInX-fOutX===================================== 
	//	fOutX: Indicates whether XON/XOFF flow control is used during transmission. If this member is TRUE, transmission stops when the XoffChar character is received and starts again 
	//	when the XonChar character is received.
	//	fInX: Indicates whether XON/XOFF flow control is used during reception. If this member is TRUE, the XoffChar character is sent when the input buffer comes within XoffLim bytes 
	//	of being full, and the XonChar character is sent when the input buffer comes within XonLim bytes of being empty.
	//==================================fInX-fOutX===================================== */
	dcb.XonChar = 0x11;
	dcb.XoffChar = 0x13;

	dcb.XonLim = 100;
	/*==================================XonLim===================================== 
	//	The minimum number of bytes in use allowed in the input buffer before flow control is activated to allow transmission by the sender. This assumes that either XON/XOFF, 
	//	RTS, or DTR input flow control is specified in the fInX, fRtsControl, or fDtrControl members.
	//==================================XonLim===================================== */
	dcb.XoffLim = 100;
	/*==================================XoffLim===================================== 
	//	The minimum number of free bytes allowed in the input buffer before flow control is activated to inhibit the sender. Note that the sender may transmit characters after 
	//	the flow control signal has been activated, so this value should never be zero. This assumes that either XON/XOFF, RTS, or DTR input flow control is specified in the fInX, 
	//	fRtsControl, or fDtrControl members. The maximum number of bytes in use allowed is calculated by subtracting this value from the size, in bytes, of the input buffer.
	//==================================XoffLim===================================== */

	dcb.fNull = FALSE; //If this member is TRUE, null bytes are discarded when received.

	dcb.ByteSize = ps->DByte; //The number of bits in the bytes transmitted and received.

	if (ps->Parity != NOPARITY) {
		dcb.fParity = TRUE;
		dcb.Parity = ps->Parity;
	} else {
		dcb.fParity = FALSE;
		dcb.Parity = NOPARITY;
	}

	dcb.StopBits = ps->StopBits;

	if (!SetCommState(m_hComm, &dcb)) { // returns the current configuration. function fails if the XonChar member of the DCB structure is equal to the XoffChar member.
		if (m_hComm != NULL)
			CloseHandle(m_hComm);
		m_hComm = NULL;
		return (FALSE);
	}

	m_fOpen = TRUE;
	return TRUE;
}

int TRS232C::SetRTS(BOOL on)
{
	/*==================================SetRTS===================================== 
	//	In DTE / DCE communication, RTS (Request to Send) is an output on the DTE and input on the DCE. CTS (Clear to Send) is the answering signal coming from the DCE. 
	//	Before sending a data, the DTE asks permission by setting its RTS output to high. No data will be sent until the DCE grants permission by using the CTS line. 
	//	The DTE uses the DTR (Data Terminal Ready) signal to indicate it is ready to accept information, whereas the DCE uses the DSR signal for the same purpose. 
	//	DTR/DSR are normally ON or OFF for the whole connection session (e.g. Off-hook), while RTS/CTS are ON or OFF for each data transmission. DCD (Data Carrier Ready) 
	//	is used by the modem when a connection has been established with remote equipment, while RI (Ring Indicator) is used by the modem to indicate a ring signal from telephone line.
	==================================SetRTS===================================== */
	int ret;
	if (on)	
		ret = EscapeCommFunction(m_hComm, SETRTS);
	else 
		ret = EscapeCommFunction(m_hComm, CLRRTS);
	return 0;
}
//================================================
//SETRTS              3       // Set RTS high	//
//CLRRTS              4       // Set RTS low	//
//SETDTR              5       // Set DTR high	//
//CLRDTR              6       // Set DTR low	//
//================================================
int TRS232C::SetDTR(BOOL on)
{
	int ret;
	if (on)
		ret = EscapeCommFunction(m_hComm, SETDTR);
	else
		ret = EscapeCommFunction(m_hComm, CLRDTR);
	return 0;
}



int TRS232C::ReadLenght(void)
{	
	/*============================COMSTAT===========================================
	//DWORD fCtsHold : 1;  // Tx waiting for CTS signal
	//DWORD fDsrHold : 1; // Tx waiting for DSR signal
	//DWORD fRlsdHold : 1; // Tx waiting for RLSD signal
	//DWORD fXoffHold : 1; // Tx waiting, XOFF char rec'd
	//DWORD fXoffSent : 1; // Tx waiting, XOFF char sent
	//DWORD fEof : 1;  // EOF character received
	//DWORD fTxim : 1; // Character waiting for Tx; char queued with TransmitCommChar
	//DWORD fReserved : 25;
	//DWORD cbInQue;  // comStat.cbInQue bytes have been received, but not read
	//DWORD cbOutQue; // comStat.cbOutQue bytes are awaiting transfer
	//==============================COMSTAT=========================================*/

	DWORD dwErrorMask;	//mask of error bits
	COMSTAT comstat;	//status structure
	int ret;


	ret = ClearCommError(m_hComm, &dwErrorMask, &comstat);

	/*============================READ===========================================
	//	cbInQue value points to the number of bytes received by the serial provider but not yet read by a  ReadFile operation. It will be zero when you haven't begin read operation.
	//	In my opinion, you don't need to check cbInQue data, the ReadFile method could do this.
	//	lpNumberOfBytesRead parameter is pointer to the variable that receives the number of bytes read when using a synchronous hFile parameter.
	//	ReadFile sets this value to zero  before doing any work or error checking. When a synchronous read operation reaches the end of a file,
	//	ReadFile returns TRUE and sets *lpNumberOfBytesRead to zero.
	//==============================READ=========================================*/

	return((int)comstat.cbInQue);
}

int TRS232C::Read(BYTE * lpbuffer, DWORD nToRead)
{
	DWORD pnByte, theLength;

	//theLength = 4;
	theLength = ReadLenght();
	if (theLength == 0) return(0);


	if (nToRead > theLength) 
		nToRead = theLength;

	ReadFile(m_hComm, lpbuffer, nToRead, &pnByte, NULL); //If the function succeeds, the return value is nonzero (TRUE).
	/*===========================================ReadFile========================================
	//	If lpOverlapped is NULL, the read operation starts at the current file position and ReadFile does not return until the operation is complete, and the system updates the file pointer before ReadFile returns.
	//	If lpOverlapped is not NULL, the read operation starts at the offset that is specified in the OVERLAPPED structure and ReadFile does not return until the read operation is complete. The system updates the OVERLAPPED offset before ReadFile returns.
	//	When a synchronous read operation reaches the end of a file, ReadFile returns TRUE and sets *lpNumberOfBytesRead to zero.
	===========================================ReadFile========================================*/
	lpbuffer[pnByte] = 0;

	return((int)pnByte);
}

void TRS232C::ClearRcvBuffer(void)
{
	if (!m_fOpen)
		return;
	if (m_fOpen == NULL)
		return;

	int theLen;

	theLen = ReadLenght();
	if (theLen < 1)
		return;

	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR); //If the function succeeds, the return value is nonzero (TRUE).
	return;
}

void TRS232C::Close(void)
{
	if (m_hComm != NULL)
		CloseHandle(m_hComm);
	m_fOpen = FALSE;
	m_hComm = NULL;
}

// int TRs 232 c :: Write (BYTE * lpBuffer, DWORD the Length)
// Argument 1: Pointer to write data
// Argument 2: Write data length
// Returned value: Number of successfully written data
// Function: Write to 232 C port
int TRS232C::Write(BYTE * lpBuffer, DWORD theLenght)
{
	DWORD ret;
	DWORD NumberOfBytesWritten;

	HANDLE hEvent;
	HANDLE hFile;
	DWORD dwReturnValue;

	ret = WriteFile(m_hComm, lpBuffer, theLenght, &NumberOfBytesWritten, NULL);

	if (ret == 0)
	{
		system_error("Writing file");
	}

	return (NumberOfBytesWritten);
}

//Error handing
void TRS232C::system_error(char * name)
{

	char *ptr = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		GetLastError(),
		0,
		(char *)&ptr,
		1024,
		NULL);

	fprintf(stderr, "\nError %s: %s\n", name, ptr);
	LocalFree(ptr);

}

LPCTSTR TRS232C::ErrorMessage(DWORD error)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	return((LPCTSTR)lpMsgBuf);
}









