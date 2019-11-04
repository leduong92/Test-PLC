#ifndef KEYENCE_VT3E_H
#define KEYENCE_VT3E_H
// ============================================================ =========================================================
// FileName: KeyenceVT3e.h
// Class: Keyence VT3e
//: KEYENCE touch panel VT3 communication
// Creation date: 2013.05.29 coba
// ============================================================ =========================================================
// Fix
// ============================================================ =========================================================
#include <winsock2.h>
#include <windows.h>

// Default port number of XG
#define VT3E_DEF_PORT 8502


class KeyenceVT3e
{
public:
KeyenceVT3e (void);
~ KeyenceVT3e (void);

BOOL Init (void);

// Word device read / write
int ReadW (const char * ipaddr, const int port, const int dev_no, const int cnt, char * buf, unsigned int bufsize);
int WriteW (const char * ipaddr, const int port, const int dev_no, const int cnt, char * dat);

// Bit device read / write
int ReadB (const char * ipaddr, const int port, const int dev_no);
int WriteB (const char * ipaddr, const int port, const int dev_no, const BOOL on);


protected:

private:
BOOL m_fOpen; // Open = TRUE, close = FALSE

int UdpSend (char * ipaddr, unsigned short port, char * data, int size, char * rcv, unsigned int rsize);
unsigned short htous (char * hexstr, short len);

// Send / receive structure
typedef struct {
	long null;
	unsigned long len;
	char cmd [3]; // 3
	char siki; // 1
	unsigned short dev_no; // 2
	unsigned char dev_cnt; // 1
} VT_UDP_SEND;

typedef struct {
	long null;
	unsigned long len;
	char cmd [3]; // 3
	char siki; // 1
	unsigned short dev_no; // 2
} VT_UDP_SEND_BIT;

typedef struct {
	long null;
	unsigned long len;
	char cmd [3]; // 3
	char result; // 1
} VT_UDP_RCV;

};
#endif