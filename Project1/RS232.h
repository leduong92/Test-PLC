#pragma once
#ifndef RS232_H

#define RS232_H

#include <windows.h>
#include <windowsx.h>

#include <wingdi.h>
#include <winspool.h>

typedef	struct {
	DWORD BauRate;
	BYTE DByte;
	BYTE StopBits;
	BYTE Parity;
	DWORD fBinary;
	DWORD fOutxCtsFlow;
	DWORD fOutxDsrFlow;
	DWORD fDtrControl;
	DWORD fRtsControl;
}PORTSETTING;

#define CR 0x0d
#define LF 0x0a
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15


class TRS232C
{
	HANDLE m_hComm;
public:
	TRS232C();
	~TRS232C();

	BOOL CommOpen(void);
	BOOL CommOpen(long comNo, PORTSETTING* ps);

	int SetRTS(BOOL on);
	int SetDTR(BOOL on);

	int ReadLenght(void);
	int Read(BYTE *buf, DWORD nToRead);

	void ClearRcvBuffer(void);
	void Close(void);

	int Write(BYTE *buf, DWORD theLenght);

	void system_error(char *name);
	LPCTSTR ErrorMessage(DWORD error);
private:
protected:
	BOOL m_fOpen;

};

#endif // RS232_H
