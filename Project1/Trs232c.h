#ifndef TRS232CR_H
#define TRS232CR_H
//========================================================================================
//
//	FileName	:TRS232C.H
//	Class		:TRs232c
// 				:RS232C��{�N���X
//	�쐬��		:1996.10.18	Coded by Ishida
//	�C����		:1999.12.06	by coba �ėp�����������邽�߁A�S�������Őݒ肵�Ă���������ύX
//	�C����		:2000.03.23	by coba ���C��������w�肵�ĵ���݂���֐���ǉ� CommOpenEx()
//
//				�f�t�H���g��CommOpen(void)���g�p���ă|�[�g���J�����ꍇ�̐ݒ�
//					�|�[�g�@�@�FCOM1
//					�ʐM���x�@�F9600bps
//					�ް��ޯā@�F8
//					Xon/Xoff�@�F����Ȃ�
//					�p���e�B�@�FEven
//					�į���ޯ� �F2
//					
//					
//========================================================================================
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <winspool.h>


typedef struct{
	DWORD BauRate;
	BYTE DByte;
	BYTE StopBits;
	BYTE Parity;
}PORTSETTING;

typedef struct{
	DWORD BauRate;			// CBR_????   ????�́A 110,300,600,1200,2400,4800,9600,14400,19200,38400,56000,57600,115200,128000,256000
	BYTE DByte;
	BYTE StopBits;
	BYTE Parity;
	DWORD fBinary;			// TRUE or FALSE
	DWORD fOutxCtsFlow;		// TRUE or FALSE
	DWORD fOutxDsrFlow;		// TRUE or FALSE
	DWORD fDtrControl;		// DTR_CONTROL_DISABLE or DTR_CONTROL_ENABLE or DTR_CONTROL_HANDSHAKE
	DWORD fRtsControl;		// RTS_CONTROL_ENABLE or RTS_CONTROL_DISABLEE or RTS_CONTROL_HANDSHAKE or RTS_CONTROL_TOGGLE
}PORTSETTINGEX;

// ASCII�R�[�h
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
	HANDLE m_hComm;						//���f���n���h��

	public:
		TRS232C();						// Constructer
		~TRS232C();						// Destructer


		BOOL CommOpen(long comNo );
		BOOL CommOpen(long comNo ,PORTSETTING* ps);
		BOOL CommOpenEx(long comNo ,PORTSETTINGEX* psex);
		BOOL CommOpen(void);
		int	Write(BYTE *buf,DWORD theLength);	//
		int	Read( BYTE *buf,DWORD nToRead);	//
		int ReadLength(void);	//
		int SetRTS(BOOL on);
		int SetDTR(BOOL on);
		int RTS_On();
		int RTS_Off();
		int DTR_On();
		int DTR_Off();
		int Status(void);
		int Status2(void);	
		int WaitLength(short Length,short WaitTime);
		void Close(void);
		void ClearRcvBuffer(void);

	protected:
		BOOL m_fOpen;
		BOOL SetComm(void);
		BOOL SetCommEx(PORTSETTING* ps);

};

#endif




