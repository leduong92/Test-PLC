//========================================================================================================
//
//	FileName	:TRS232CR2.CPP
//	Class		:TRS232C2
// 				:RS232C基本クラス（古いＡＰＩを利用しＲＴＳ,ＤＴＲを制御する）
//	作成日		:1998. 2.13	Coded by Ishida
//	修正日		:1998. 2.26 Shiba
//	修正日		:1999.12.06	by coba 汎用性を持たせるため、ゴリ書きで設定していた部分を変更
//
//========================================================================================================
#include "TRS232C2.h"
#include <stdio.h>
#include <stdlib.h>

//========================================================================================================
// TRs232c2::TRs232c2()
//	コンストラクタ(デフォルト)
//	引数：なし
//	返値：なし
//
//========================================================================================================
TRS232C2::TRS232C2()	//Constructer
{
	m_hComm=NULL;
	m_fOpen=FALSE;
}

 
//========================================================================================================
// TRS232C2::Open(LPCTSTR theComm)	//Constructer
//	ポートオープン(デフォルト設定)。ごく一般的な設定でポート１を開く
//	引数：なし
//	返値：TRUE or FALSE
//========================================================================================================
BOOL TRS232C2::CommOpen(void)
{
	return(CommOpen(1));
}

//========================================================================================================
// TRS232C2::Open(LPCTSTR theComm)	//Constructer
//	ポートオープン
//	引数：ｵｰﾌﾟﾝするポート番号(1=com1,2=com2...)
//	返値：TRUE or FALSE
//	232cポートのオープン
//========================================================================================================
BOOL TRS232C2::CommOpen(long comNo)
{
	return(CommOpen(comNo,NULL));
}
BOOL TRS232C2::CommOpen(long comNo ,PORTSETTING* ps)
{
	// 既に開いてるときは無視
	if( m_fOpen ) return(FALSE);

	// 引数チェック
	if( comNo<1 ) return(FALSE);

	// ポート名の作成
	char theComm[20];
	wsprintf( theComm, "COM%ld", comNo );


	DWORD	fdwAccess ,fdwShareMode,fSendBuffer=4096,fReceiveBuffer=4096;
	LPSECURITY_ATTRIBUTES	lpsa;
	DWORD	fdwCreate ,fdwAttrsAndFlags;
	HANDLE	hTemplateFile ;
	COMMTIMEOUTS  CommTimeOuts ;


	fdwAccess = GENERIC_READ | GENERIC_WRITE;	// モデムへのアクセスタイプ( 読み｜書き )
	fdwShareMode = 0;							// モデムの共有属性（共有禁止）
	lpsa = NULL;								// モデムハンドルの継承セキュリティ（継承禁止）
	fdwCreate = OPEN_EXISTING ;					// モデムが存在する（存在してもエラーがでない）
	fdwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL;
	hTemplateFile = NULL;

	// モデムのオープン
	m_hComm=CreateFile(theComm,fdwAccess,fdwShareMode,lpsa,fdwCreate,fdwAttrsAndFlags,hTemplateFile);

	// ｵｰﾌﾟﾝ失敗
	if( m_hComm == INVALID_HANDLE_VALUE ){
		m_hComm=NULL;
		return(FALSE);
	}

	// 待ちたいイベントの指定（ 0 = disables all events ）
	SetCommMask( m_hComm,0 ) ;

	// デバイスの初期化
	SetupComm(
		m_hComm,	// handle of communications device  
		fSendBuffer,	// size of input buffer
		fReceiveBuffer	// size of output buffer
	);

	// 保留状態の読み書き処理を異常終了させる
    PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	// set up for overlapped I/O
	// タイムアウトの設定
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 5 ;
	CommTimeOuts.WriteTotalTimeoutConstant = 1500 ;
	SetCommTimeouts( m_hComm, &CommTimeOuts ) ;

	// 232c通信設定(default)
	if( ps==NULL){
		if(!SetComm()){
			if(m_hComm!=NULL)
				CloseHandle(m_hComm);
			m_hComm=NULL;
			return(FALSE);
		}
	}
	// 指定あり
	else{
		if(!SetCommEx(ps)){
			if(m_hComm!=NULL)
				CloseHandle(m_hComm);
			m_hComm=NULL;
			return(FALSE);
		}
	}

	// openフラグ
	m_fOpen=TRUE;
	return(TRUE);
}


//========================================================================================================
//	TRs232c2::~TRs232c()
//	デストラクタ
//	引数：なし
//	返値：なし
//	(開いていたら)２３２ｃポートのクロース
//========================================================================================================
TRS232C2::~TRS232C2()	//Destructer
{

	if(m_hComm!=NULL)
		CloseHandle(m_hComm);
	m_fOpen=FALSE;

}

//========================================================================================================
//	void TRS232C2::Close(void)
//	ポートクローズ
//	引数：なし
//	返値：なし
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
//	type：private
//	引数：なし
//	返値：なし
//	通信仕様の設定 232cの速度等のセット
//========================================================================================================
BOOL TRS232C2::SetComm(void)
{
	// CBR_9600 is approximately 1byte/ms. For our purposes, allow
	// double the expected time per character for a fudge factor.

	DCB dcb;
	GetCommState(m_hComm,&dcb);
	//通信速度変更時ここをいじる
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
//	type：private
//	引数：なし
//	返値：なし
//	通信仕様の設定 232cの速度等のセット
//========================================================================================================
BOOL TRS232C2::SetCommEx(PORTSETTING* ps)
{
	// CBR_9600 is approximately 1byte/ms. For our purposes, allow
	// double the expected time per character for a fudge factor.

	DCB dcb;
	GetCommState(m_hComm,&dcb);
	//通信速度変更時ここをいじる
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
//	引数：なし
//	返値：なし
//	受信バッファのクリア
//========================================================================================================
void TRS232C2::ClearRcvBuffer(void)
{
	if( !m_fOpen ) return;
	if( m_hComm==NULL ) return;

	// 何ﾊﾞｲﾄ来てるか
	int theLen;

	theLen = ReadLength();
	if( theLen < 1 ) return;		// ﾃﾞｰﾀなし

	// ある場合は捨てる
    PurgeComm( m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	return;

}

//========================================================================================================
//	int TRs232c2::RTS_On()
//	引数：なし
//	返値：なし
//	ＲＴＳ信号（送信要求信号）をONする。
//========================================================================================================
int TRS232C2::RTS_On()
{
	EscapeCommFunction(m_hComm,SETRTS);

	return(0);
}
//========================================================================================================
//	int TRs232c2::RTS_Off()
//	引数：なし
//	返値：なし
//	ＲＴＳ信号（送信要求信号）をOFFする。
//========================================================================================================
int TRS232C2::RTS_Off()
{
	EscapeCommFunction(m_hComm,CLRRTS);

	return(0);
}

//========================================================================================================
//	int TRs232c2::DTR_On()
//	引数：なし
//	返値：なし
//	ＤＴＲ信号（ﾃﾞｰﾀﾀｰﾐﾅﾙﾚﾃﾞｨ信号）をONする。
//========================================================================================================
int TRS232C2::DTR_On()
{
	EscapeCommFunction(m_hComm,SETDTR);

	return(0);
}
//========================================================================================================
//	int TRs232c2::DTR_Off()
//	引数：なし
//	返値：なし
//	ＤＴＲ信号（ﾃﾞｰﾀﾀｰﾐﾅﾙﾚﾃﾞｨ信号）をOFFする。
//========================================================================================================
int TRS232C2::DTR_Off()
{
	EscapeCommFunction(m_hComm,CLRDTR);
	return(0);

}

//========================================================================================================
//	int TRs232c2::Write(BYTE *lpBuffer,DWORD theLength)
//	引数１：書き込みデータのポインタ
//	引数２：書き込みデータ長さ
//	返値　：書き込み成功データ数
//　機能　：232Cポートへの書き込み
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
//	引数１：書き込みデータのポインタ
//	引数２：書き込みデータ長さ
//	返値　：読み込みデータバイト数
//　機能　：232Cポートからのﾃﾞｰﾀ読み出し
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
//	引数　：なし
//	返値　：データバイト数
//　機能　：現在232C受信ポートに溜まっているデータバイト数を調べる
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
//	引数　：なし
//	返値　：データバイト数
//　機能　：現在232C受信ポートに溜まっているデータバイト数を調べる
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
//	引数　：なし
//	返値　：データバイト数
//　機能　：現在232C受信ポートに溜まっているデータバイト数を調べる
//========================================================================================================
int TRS232C2::Status2(void)	//
{
	DWORD ModemStat;	//

	GetCommModemStatus(m_hComm,&ModemStat);


	return((int)ModemStat);
}


//========================================================================================================
//	int TRs232c2::	
//	引数　：なし
//	返値　：データバイト数
//　機能　：現在232C受信ポートに溜まっているデータバイト数を調べる
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