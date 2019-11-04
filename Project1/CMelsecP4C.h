#pragma once

#ifndef COM_PLC4EX_H
#define COM_PLC4EX_H

#include "RS232.h"

const int P4_HEADLEN1 = 14;

class CMelsecP4C : public TRS232C
{
public:
	CMelsecP4C();
	~CMelsecP4C();

	BOOL Init(int portNo, int baudrate, int databit, int stopbit, int parity, int wait, BOOL ctrl_line = TRUE);
	void ComClose(void);

	int P4WriteB(char* station, char* pcnumber, char* address, int onoff);
	int P4ReadB(char* station, char* pcnumber, char* address);

	int P4WriteW(char* station, char* pcstation, char* address, short counts, char* msg);
	int P4ReadW(char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts);

	void MakeCmd(char* buf, int bufsize, char* station, char* pc, char* cmd, char* address, short counts, char* msg);
	void MakeCmd2C(char* buf, int bufsize, char* station, char* pc, char* cmd, char* addr, short counts, char*msg);

	int GetSumCheckCode(char* str);
	//ENQ(1) + ST(2) + PC(2) + CMD(2) + WAIT(1) + SUM(2) + numbers of devices(2) + CRLF(2)

	int Receive(char* buf, int bufsize, int waitlen);

	int Answer(char flg, char * station, char * pc);

	int P4ReadBB(char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts);
	int P4WriteBB(char* station, char* pcstation, char* addr, int cnt, char* dat);

	int P2C4WriteW(char* station, char* pcstation, char* addr, short counts, char* msg);
	int P2C4ReadW(char* buf, int bufsize, char* station, char* pcstation, char* addr, short counts);

private:
	BOOL m_fOpen;
	int m_wait;

	char m_buf[4096];
};


#endif // !COM_PLC4EX_H