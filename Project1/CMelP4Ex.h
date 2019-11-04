#ifndef CMelsecP4Ex
#define CMelsecP4Ex_H

#include "RS232.h"

const int P4_HEADLEN=14;

class CMelsecP4Ex:public TRS232C
{
	public:
		CMelsecP4Ex( void );
		~CMelsecP4Ex( void );

		BOOL Init(int portNo,int baudrate,int databit,int stopbit,int parity,int wait,BOOL ctrl_line=TRUE);
		void ComClose(void);	

		int P4WriteW( char* station, char* pcstation,char* addr, short counts, char* msg);
		int P4ReadW( char* buf,int bufsize,char* station,char* pcstation,char* addr, short counts);

		int P2C4WriteW( char* station, char* pcstation,char* addr, short counts, char* msg);
		int P2C4ReadW( char* buf,int bufsize,char* station,char* pcstation,char* addr, short counts);

		int P4Send( char* station, char* pcstation,char* command,char* addr, short counts, char* msg);

		int P4WriteB( char* station, char* pcstation,char* addr, int onoff);
		int P4ReadB( char* station,char* pcstation,char* addr);
		int P4ReadBB( char* buf,int bufsize,char* station,char* pcstation,char* addr,short counts);
		int P4WriteBB( char* station, char* pcstation,char* addr, int cnt,char* dat);

		int Str2Mel(char* buf, unsigned int bufsize, char* str);
		int Mel2Str(char* buf, unsigned int bufsize, char* melstr);
		int UStr2Mel(char* buf, unsigned int bufsize, unsigned char* str);

		BOOL ULChg( char* buf );

	protected:

	private:
		BOOL m_fOpen;					
		int m_Wait;

		int GetCheckSum(char* str );

		void MakeCmd(char* buf,int bufsize,char* station,char* pc,char* cmd,char* addr,short counts,char*msg);
		void MakeCmd2C(char* buf,int bufsize,char* station,char* pc,char* cmd,char* addr,short counts,char*msg);

		int Receive( char* buf, int bufsize, int waitlen );
		int Answer( char flg, char* station,char* pc );

		char m_buf[4096];

		unsigned char htoi( char *hexstr , short len );

};
#endif
