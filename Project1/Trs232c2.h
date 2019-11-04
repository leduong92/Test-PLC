#ifndef TRS232C2_H
#define TRS232C2_H
//========================================================================================
//
//	FileName	:TRS232C2.H
//	Class		:TRs232c2
// 				:RS232C2基本クラス
//	作成日		:1996.10.18	Coded by Ishida
//	修正日		:1999.12.06	by coba 汎用性を持たせるため、ゴリ書きで設定していた部分を変更
//
//				デフォルトのCommOpen(void)を使用してポートを開いた場合の設定
//					ポート　　：COM1
//					通信速度　：9600bps
//					ﾃﾞｰﾀﾋﾞｯﾄ　：8
//					Xon/Xoff　：制御なし
//					パリティ　：Even
//					ｽﾄｯﾌﾟﾋﾞｯﾄ ：2
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

// ASCIIコード
#define CR 0x0d
#define LF 0x0a

class TRS232C2
{
	HANDLE m_hComm;						//モデムハンドル

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