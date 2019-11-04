//========================================================================================================
//
//	FileName	:TRS232CR2.CPP
//	Class		:TRS232C2
// 				:RS232C��{�N���X�i�Â��`�o�h�𗘗p���q�s�r,�c�s�q�𐧌䂷��j
//	�쐬��		:1998. 2.13	Coded by Ishida
//	�C����		:1998. 2.26 Shiba
//	�C����		:1999.12.06	by coba �ėp�����������邽�߁A�S�������Őݒ肵�Ă���������ύX
//
//========================================================================================================
#include "TRS232C2.h"
#include <stdio.h>
#include <stdlib.h>

//========================================================================================================
// TRs232c2::TRs232c2()
//	�R���X�g���N�^(�f�t�H���g)
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//
//========================================================================================================
TRS232C2::TRS232C2()	//Constructer
{
	m_hComm=NULL;
	m_fOpen=FALSE;
}

 
//========================================================================================================
// TRS232C2::Open(LPCTSTR theComm)	//Constructer
//	�|�[�g�I�[�v��(�f�t�H���g�ݒ�)�B������ʓI�Ȑݒ�Ń|�[�g�P���J��
//	�����F�Ȃ�
//	�Ԓl�FTRUE or FALSE
//========================================================================================================
BOOL TRS232C2::CommOpen(void)
{
	return(CommOpen(1));
}

//========================================================================================================
// TRS232C2::Open(LPCTSTR theComm)	//Constructer
//	�|�[�g�I�[�v��
//	�����F����݂���|�[�g�ԍ�(1=com1,2=com2...)
//	�Ԓl�FTRUE or FALSE
//	232c�|�[�g�̃I�[�v��
//========================================================================================================
BOOL TRS232C2::CommOpen(long comNo)
{
	return(CommOpen(comNo,NULL));
}
BOOL TRS232C2::CommOpen(long comNo ,PORTSETTING* ps)
{
	// ���ɊJ���Ă�Ƃ��͖���
	if( m_fOpen ) return(FALSE);

	// �����`�F�b�N
	if( comNo<1 ) return(FALSE);

	// �|�[�g���̍쐬
	char theComm[20];
	wsprintf( theComm, "COM%ld", comNo );


	DWORD	fdwAccess ,fdwShareMode,fSendBuffer=4096,fReceiveBuffer=4096;
	LPSECURITY_ATTRIBUTES	lpsa;
	DWORD	fdwCreate ,fdwAttrsAndFlags;
	HANDLE	hTemplateFile ;
	COMMTIMEOUTS  CommTimeOuts ;


	fdwAccess = GENERIC_READ | GENERIC_WRITE;	// ���f���ւ̃A�N�Z�X�^�C�v( �ǂ݁b���� )
	fdwShareMode = 0;							// ���f���̋��L�����i���L�֎~�j
	lpsa = NULL;								// ���f���n���h���̌p���Z�L�����e�B�i�p���֎~�j
	fdwCreate = OPEN_EXISTING ;					// ���f�������݂���i���݂��Ă��G���[���łȂ��j
	fdwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL;
	hTemplateFile = NULL;

	// ���f���̃I�[�v��
	m_hComm=CreateFile(theComm,fdwAccess,fdwShareMode,lpsa,fdwCreate,fdwAttrsAndFlags,hTemplateFile);

	// ����ݎ��s
	if( m_hComm == INVALID_HANDLE_VALUE ){
		m_hComm=NULL;
		return(FALSE);
	}

	// �҂������C�x���g�̎w��i 0 = disables all events �j
	SetCommMask( m_hComm,0 ) ;

	// �f�o�C�X�̏�����
	SetupComm(
		m_hComm,	// handle of communications device  
		fSendBuffer,	// size of input buffer
		fReceiveBuffer	// size of output buffer
	);

	// �ۗ���Ԃ̓ǂݏ����������ُ�I��������
    PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	// set up for overlapped I/O
	// �^�C���A�E�g�̐ݒ�
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 5 ;
	CommTimeOuts.WriteTotalTimeoutConstant = 1500 ;
	SetCommTimeouts( m_hComm, &CommTimeOuts ) ;

	// 232c�ʐM�ݒ�(default)
	if( ps==NULL){
		if(!SetComm()){
			if(m_hComm!=NULL)
				CloseHandle(m_hComm);
			m_hComm=NULL;
			return(FALSE);
		}
	}
	// �w�肠��
	else{
		if(!SetCommEx(ps)){
			if(m_hComm!=NULL)
				CloseHandle(m_hComm);
			m_hComm=NULL;
			return(FALSE);
		}
	}

	// open�t���O
	m_fOpen=TRUE;
	return(TRUE);
}


//========================================================================================================
//	TRs232c2::~TRs232c()
//	�f�X�g���N�^
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	(�J���Ă�����)�Q�R�Q���|�[�g�̃N���[�X
//========================================================================================================
TRS232C2::~TRS232C2()	//Destructer
{

	if(m_hComm!=NULL)
		CloseHandle(m_hComm);
	m_fOpen=FALSE;

}

//========================================================================================================
//	void TRS232C2::Close(void)
//	�|�[�g�N���[�Y
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//========================================================================================================
void TRS232C2::Close(void)
{
	if(m_hComm!=NULL)
		CloseHandle(m_hComm);
	m_fOpen=FALSE;
	m_hComm=NULL;
}

//========================================================================================================
//	BOOL SetComm(void)
//	type�Fprivate
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	�ʐM�d�l�̐ݒ� 232c�̑��x���̃Z�b�g
//========================================================================================================
BOOL TRS232C2::SetComm(void)
{
	// CBR_9600 is approximately 1byte/ms. For our purposes, allow
	// double the expected time per character for a fudge factor.

	DCB dcb;
	GetCommState(m_hComm,&dcb);
	//�ʐM���x�ύX��������������
	dcb.BaudRate = CBR_19200;// current baud rate 
	dcb.BaudRate = CBR_9600;// current baud rate 
	dcb.BaudRate = CBR_1200;// current baud rate 

	dcb.fBinary = TRUE;	// binary mode, no EOF check

//	dcb.fOutxCtsFlow = TRUE;
	dcb.fOutxCtsFlow = FALSE;
//	dcb.fOutxDsrFlow = TRUE;
	dcb.fOutxDsrFlow = FALSE;

	dcb.fDtrControl = DTR_CONTROL_DISABLE;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;

	dcb.fRtsControl = RTS_CONTROL_DISABLE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
//	dcb.fRtsControl = RTS_CONTROL_ENABLE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
//	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
//	dcb.fRtsControl = RTS_CONTROL_TOGGLE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE

//	dcb.fDsrSensitivity = FALSE;


	// Xon/Xoff
	dcb.fTXContinueOnXoff= FALSE ;
	dcb.fInX = dcb.fOutX = FALSE ;
	dcb.XonChar = 0x11 ;
	dcb.XoffChar = 0x13 ;
	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ;

	dcb.fNull= FALSE ;
	// Data Bits
	dcb.ByteSize = 7;

	// Parity Check & parity bits
	dcb.fParity = FALSE;
//	dcb.fParity = TRUE;

	dcb.Parity =NOPARITY;
//	dcb.Parity =EVENPARITY;
//	dcb.Parity =ODDPARITY;

	// Stop bit
	dcb.StopBits = ONESTOPBIT;	//ONESTOPBITS
//	dcb.StopBits = TWOSTOPBITS;		//TWOSTOPBITS


	if(!SetCommState(m_hComm,&dcb)) return(FALSE);
	return(TRUE);
}

//========================================================================================================
//	BOOL SetComm(void)
//	type�Fprivate
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	�ʐM�d�l�̐ݒ� 232c�̑��x���̃Z�b�g
//========================================================================================================
BOOL TRS232C2::SetCommEx(PORTSETTING* ps)
{
	// CBR_9600 is approximately 1byte/ms. For our purposes, allow
	// double the expected time per character for a fudge factor.

	DCB dcb;
	GetCommState(m_hComm,&dcb);
	//�ʐM���x�ύX��������������
	dcb.BaudRate = ps->BauRate;// current baud rate 

	dcb.fBinary = TRUE;	// binary mode, no EOF check

//	dcb.fOutxCtsFlow = TRUE;
	dcb.fOutxCtsFlow = FALSE;
//	dcb.fOutxDsrFlow = TRUE;
	dcb.fOutxDsrFlow = FALSE;
/*
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
//	dcb.fDtrControl = DTR_CONTROL_ENABLE;
//	dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;

	dcb.fRtsControl = RTS_CONTROL_DISABLE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
//	dcb.fRtsControl = RTS_CONTROL_ENABLE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
//	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
//	dcb.fRtsControl = RTS_CONTROL_TOGGLE; //RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLEE
*/
	dcb.fDtrControl = ps->fDtrControl;
	dcb.fRtsControl = ps->fRtsControl;

//	dcb.fDsrSensitivity = FALSE;


	// Xon/Xoff
	dcb.fTXContinueOnXoff= FALSE ;
	dcb.fInX = dcb.fOutX = FALSE ;
	dcb.XonChar = 0x11 ;
	dcb.XoffChar = 0x13 ;
	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ;

	dcb.fNull= FALSE ;
	// Data Bits
	dcb.ByteSize = ps->DByte;

	// Parity Check & parity bits
	if( ps->Parity !=NOPARITY){
		dcb.fParity = TRUE;
		dcb.Parity =ps->Parity;
	}
	else{
		dcb.fParity = FALSE;
		dcb.Parity =NOPARITY;
	}

	// Stop bit
	dcb.StopBits = ps->StopBits;	//ONESTOPBITS
//	dcb.StopBits = TWOSTOPBITS;		//TWOSTOPBITS


	if(!SetCommState(m_hComm,&dcb)) return(FALSE);
	return(TRUE);
}

//========================================================================================================
//	void TRS232C2::ClearRcvBuffer(void)
//	Type:public
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	��M�o�b�t�@�̃N���A
//========================================================================================================
void TRS232C2::ClearRcvBuffer(void)
{
	if( !m_fOpen ) return;
	if( m_hComm==NULL ) return;

	// ���޲ė��Ă邩
	int theLen;

	theLen = ReadLength();
	if( theLen < 1 ) return;		// �ް��Ȃ�

	// ����ꍇ�͎̂Ă�
    PurgeComm( m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	return;

}

//========================================================================================================
//	int TRs232c2::RTS_On()
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	�q�s�r�M���i���M�v���M���j��ON����B
//========================================================================================================
int TRS232C2::RTS_On()
{
	EscapeCommFunction(m_hComm,SETRTS);

	return(0);
}
//========================================================================================================
//	int TRs232c2::RTS_Off()
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	�q�s�r�M���i���M�v���M���j��OFF����B
//========================================================================================================
int TRS232C2::RTS_Off()
{
	EscapeCommFunction(m_hComm,CLRRTS);

	return(0);
}

//========================================================================================================
//	int TRs232c2::DTR_On()
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	�c�s�q�M���i�ް��������ި�M���j��ON����B
//========================================================================================================
int TRS232C2::DTR_On()
{
	EscapeCommFunction(m_hComm,SETDTR);

	return(0);
}
//========================================================================================================
//	int TRs232c2::DTR_Off()
//	�����F�Ȃ�
//	�Ԓl�F�Ȃ�
//	�c�s�q�M���i�ް��������ި�M���j��OFF����B
//========================================================================================================
int TRS232C2::DTR_Off()
{
	EscapeCommFunction(m_hComm,CLRDTR);
	return(0);

}

//========================================================================================================
//	int TRs232c2::Write(BYTE *lpBuffer,DWORD theLength)
//	�����P�F�������݃f�[�^�̃|�C���^
//	�����Q�F�������݃f�[�^����
//	�Ԓl�@�F�������ݐ����f�[�^��
//�@�@�\�@�F232C�|�[�g�ւ̏�������
//========================================================================================================
int TRS232C2::Write(BYTE *lpBuffer,DWORD theLength)
{
	DWORD	ret;
//	DWORD	err4;
	DWORD  NumberOfBytesWritten;

	ret=WriteFile(m_hComm,lpBuffer,theLength,&NumberOfBytesWritten,NULL);
	if(ret==0 )
	{
//		err4=GetLastError();
//		WinPrintf("Write Err=%ld\n",err4);
	}

//	WinPrintf("ret=%ld send =%02x NumberOfBytesWritten=%d \n",theLength,lpBuffer[0],NumberOfBytesWritten);
	return(NumberOfBytesWritten);
}

//========================================================================================================
//	int TRs232c2::Read(BYTE *lpBuffer,DWORD nToRead)	
//	�����P�F�������݃f�[�^�̃|�C���^
//	�����Q�F�������݃f�[�^����
//	�Ԓl�@�F�ǂݍ��݃f�[�^�o�C�g��
//�@�@�\�@�F232C�|�[�g������ް��ǂݏo��
//========================================================================================================
int TRS232C2::Read(BYTE *lpBuffer,DWORD nToRead)
{
	DWORD pnByte,theLength;
	
	theLength = ReadLength();
	if(theLength==0) return(0);
	

	if( nToRead > theLength ) nToRead = theLength;
	
	ReadFile(m_hComm,lpBuffer,nToRead,&pnByte,NULL);
	lpBuffer[pnByte]=0;

	//printf("\nlpBuffer=%s,byte=%d,nToR=%d",lpBuffer,pnByte,nToRead);

	return((int)pnByte);
	
}


//========================================================================================================
//	int TRs232c2::	
//	�����@�F�Ȃ�
//	�Ԓl�@�F�f�[�^�o�C�g��
//�@�@�\�@�F����232C��M�|�[�g�ɗ��܂��Ă���f�[�^�o�C�g���𒲂ׂ�
//========================================================================================================
int TRS232C2::ReadLength(void)	//
{
	DWORD dwErrorMask;	//mask of error bits
	COMSTAT comstat;	//status structure


	ClearCommError(m_hComm,&dwErrorMask,&comstat);


	return((int)comstat.cbInQue);
}
//========================================================================================================
//	int TRs232c2::	
//	�����@�F�Ȃ�
//	�Ԓl�@�F�f�[�^�o�C�g��
//�@�@�\�@�F����232C��M�|�[�g�ɗ��܂��Ă���f�[�^�o�C�g���𒲂ׂ�
//========================================================================================================
int TRS232C2::Status(void)	//
{
	DWORD dwErrorMask;	//mask of error bits
	COMSTAT comstat;	//status structure
	DWORD ModemStat;	//

	ClearCommError(m_hComm,&dwErrorMask,&comstat);

	GetCommModemStatus(m_hComm,&ModemStat);

	return((int)comstat.cbInQue);
}
//========================================================================================================
//	int TRs232c2::	
//	�����@�F�Ȃ�
//	�Ԓl�@�F�f�[�^�o�C�g��
//�@�@�\�@�F����232C��M�|�[�g�ɗ��܂��Ă���f�[�^�o�C�g���𒲂ׂ�
//========================================================================================================
int TRS232C2::Status2(void)	//
{
	DWORD ModemStat;	//

	GetCommModemStatus(m_hComm,&ModemStat);


	return((int)ModemStat);
}


//========================================================================================================
//	int TRs232c2::	
//	�����@�F�Ȃ�
//	�Ԓl�@�F�f�[�^�o�C�g��
//�@�@�\�@�F����232C��M�|�[�g�ɗ��܂��Ă���f�[�^�o�C�g���𒲂ׂ�
//========================================================================================================
int TRS232C2::WaitLength(short Length,short WaitTime)	//
{

	short counter=0,theLen=0;

	WaitTime /=10;

	while( theLen < Length )
	{
		Sleep(10);
		if( ++counter > WaitTime ) break;	
		theLen = ReadLength();
	}
	
	return (theLen);

}

