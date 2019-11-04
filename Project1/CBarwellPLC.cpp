#include <stdio.h>
#include "CBarwellPLC.h"

CBarwellPLC::CBarwellPLC(void)
{
	memset(m_Station, 0, sizeof(m_Station));
	memset(m_PcStation, 0, sizeof(m_PcStation));
}

CBarwellPLC::~CBarwellPLC()
{
	TRS232C::Close();
}

BOOL CBarwellPLC::Connect(int port, int baudrate, int databit, int stopbit, int parity, int wait)
{
	// Set Station#
	memset(m_Station, 0, sizeof(m_Station));
	memcpy(m_Station, "00", 2);
	memset(m_PcStation, 0, sizeof(m_PcStation));
	memcpy(m_PcStation, "FF", 2);

	//Open RS232C
	if (CMelsecP4C::Init(port, baudrate, databit, stopbit, parity, wait) == FALSE)
		return FALSE;

	return TRUE;
}
void CBarwellPLC::DisConnect(void)
{
	TRS232C::Close();
}

int CBarwellPLC::ReadTest(void)
{
	char data[256], tmp[10];
	int ret, i;

	// 1.using protocol4 (FX,A,Q,QnA)
	// Read word device from D0020 to D0022 (3 registers).
	printf("\nread D0020-23 using P4ReadW()...");
	memset(data, 0, 100);
	ret = CMelsecP4C::P4ReadW(data, sizeof(data), m_Station, m_PcStation, "D0020", 3);
	// return zero if success 
	if (!ret) {
		printf("ok!\n");
		// display received data
		memset(tmp, 0, sizeof(tmp));
		for (i = 0; i<3; i++) {
			memcpy(tmp, data + i * 4, 4);
			printf("D%d:%s,", 20 + i, tmp);
		}
	}
	// failed
	else {
		printf("err!! ret=%d", ret);
		return(1);
	}
	// 2.using 2C frame(protocol4),2C frame can access higer-address.
	// If your PLC model is FX,this protocol not support.
	// Read word device from D12020 to D120022 (3 registers).
	ret = CMelsecP4C::P2C4ReadW(data, sizeof(data), m_Station, m_PcStation, "D*012020", 3);
	printf("\n\nread D12020-12023 using P2C4ReadW()...");
	// return zero if success 
	if (!ret) {
		printf("ok!\n");
		// display received data
		memset(tmp, 0, sizeof(tmp));
		for (i = 0; i<3; i++) {
			memcpy(tmp, data + i * 4, 4);
			printf("D%d:%s,", 12020 + i, tmp);
		}
	}
	// failed
	else {
		printf("err!! ret=%d", ret);
		return(2);
	}

	// Access bit device M0001
	printf("\nread bit device M0001 using P4ReadB()...");
	ret = CMelsecP4C::P4ReadB(m_Station, m_PcStation, "M0001");
	// P4ReadB() returns zero if bit is off
	if (!ret) {
		printf("ok!,BIT=OFF");
	}
	// P4ReadB() returns 1 if bit is ON
	else if (ret == 1) {
		printf("ok!,BIT=ON");
	}
	// commuication error.
	else {
		printf("err!,ret=%d", ret);
		return(3);
	}

	return(0);
}

int CBarwellPLC::WriteTest(void)
{
	int ret;
	char send[20];

	// 1. write word device using protocol4 (FX,A,Q,QnA)
	// send '0123' to D0023,'4567' to D0024,total 2 registers
	sprintf(send, "01234567\0");		// build send data,2registers = 4*2=8 bytes
	printf("\n\nwrite D0023-24 using P4WriteW(),send=%s...", send);
	ret = CMelsecP4C::P4WriteW(m_Station, m_PcStation, "D0023", 2, send);
	// return zero if success 
	if (!ret) {
		printf("ok!\n");
	}
	else {
		printf("err!! ret=%d");
		return(1);
	}

	// 2. write word device using 2C frame (protocol4)
	// send 'ABCD' to D10023,'EF01' to D10024,total 2 registers
	sprintf(send, "ABCDEF01\0");		// build send data,2registers = 4*2=8 bytes
	printf("\nwrite D10023-10024 using P2C4WriteW(),send=%s...", send);
	ret = CMelsecP4C::P2C4WriteW(m_Station, m_PcStation, "D*010020", 2, send);
	// return zero if success 
	if (!ret) {
		printf("ok!\n");
	}
	else {
		printf("err!! ret=%d");
		return(2);
	}

	// 3.write bit device using protocol 4
	// set M0013 on
	printf("\nset M0013 ON,P4WriteB()...");
	ret = CMelsecP4C::P4WriteB(m_Station, m_PcStation, "M0013", 1);	// last parametr, set 0(off) or 1 (on)
																		// return zero if success 
	if (!ret) {
		printf("ok!\n");
	}
	else {
		printf("err!! ret=%d");
		return(2);
	}
	// set M0014 off
	printf("\nset M0014 OFF,P4WriteB()...");
	ret = CMelsecP4C::P4WriteB(m_Station, m_PcStation, "M0014", 0);	// last parametr, set 0(off) or 1 (on)
	if (!ret) {
		printf("ok!\n");
	}
	else {
		printf("err!! ret=%d");
		return(2);
	}

	return(0);

}

int CBarwellPLC::SendBarwellFPOK(void)
{
	int ret;
	char send[20];

	// 3.write bit device using protocol 4
	// set M0210 on
	printf("\nset M0210 ON,P4WriteB()...");
	ret = CMelsecP4C::P4WriteB(m_Station, m_PcStation, "M0210", 1);	// last parametr, set 0(off) or 1 (on)
																		// return zero if success 
	if (!ret) {
		printf("ok!\n");
	}
	else {
		printf("err!! ret=%d");
		return(2);
	}
	return(0);
}


int CBarwellPLC::SendBarwellFPNG(void)
{
	int ret;
	char send[20];

	// 3.write bit device using protocol 4
	// set M0210 off
	printf("\nset M0210 OFF,P4WriteB()...");
	ret = CMelsecP4C::P4WriteB(m_Station, m_PcStation, "M0210", 0);	// last parametr, set 0(off) or 1 (on)
																		// return zero if success 
	if (!ret) {
		printf("ok!\n");
	}
	else {
		printf("err!! ret=%d");
		return(2);
	}
	return(0);
}
