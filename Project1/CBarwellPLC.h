#pragma once
#ifndef BARWEL_PLC_H
#define BARWEL_PLC_H

#include <windows.h>
#include "CMelsecP4C.h"
#define MAX_PLC_STATION 99
//To access MITUBISHI PLC using Protocol-4

class CBarwellPLC : public CMelsecP4C
{
public:
	CBarwellPLC();
	~CBarwellPLC();

	BOOL Connect(int port, int baudrate, int databit, int stopbit, int parity, int wait);

	int ReadTest(void);
	int WriteTest(void);
	int SendBarwellFPOK(void);
	int SendBarwellFPNG(void);

	void DisConnect(void);
private:
	char m_Station[2 + 1];		// STATION#
	char m_PcStation[2 + 1];		// PC-STATION#
};



#endif