#ifndef TRS232C2_H
#define TRS232C2_H
//========================================================================================
//
//	FileName	:TRS232C2.H
//	Class		:TRs232c2
// 				:RS232C2��{�N���X
//	�쐬��		:1996.10.18	Coded by Ishida
//	�C����		:1999.12.06	by coba �ėp�����������邽�߁A�S�������Őݒ肵�Ă���������ύX
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
	BYTE fDtrControl;
	BYTE fRtsControl;
}PORTSETTING;

// ASCII�R�[�h
#define CR 0x0d
#define LF 0x0a

class TRS232C2
{
	HANDLE m_hComm;						//���f���n���h��

	public:
		TRS232C2();						// Constructer
		~TRS232C2();						// Destructer


		BOOL CommOpen(long comNo );
		BOOL CommOpen(long comNo ,PORTSETTING* ps);
		BOOL CommOpen(void);
		int	Write(BYTE *buf,DWORD theLength);	//
		int	Read( BYTE *buf,DWORD nToRead);	//
		int ReadLength(void);	//
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