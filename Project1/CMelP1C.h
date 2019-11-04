#ifndef COM_PLC1C_H
#define COM_PLC1C_H

#include "RS232.h"


const int P4_HEADLEN=14;

class CMelsecP1C:public TRS232C
{
	public:
		CMelsecP1C( void );
		~CMelsecP1C( void );

		BOOL Init(int portNo,int baudrate,int databit,int stopbit,int parity,int wait);

		int P4WriteW( char* station, char* pcstation,char* addr, short counts, char* msg);
		int P4ReadW( char* buf,int bufsize,char* station,char* pcstation,char* addr, short counts);


		int P4Send( char* station, char* pcstation,char* command,char* addr, short counts, char* msg);

		int P4WriteB( char* station, char* pcstation,char* addr, int onoff);
		int P4ReadB( char* station,char* pcstation,char* addr);

		int Str2Mel(char* buf, unsigned int bufsize, char* str);
		int Mel2Str(char* buf, unsigned int bufsize, char* melstr);

		BOOL ULChg( char* buf );

	protected:

	private:
		BOOL m_fOpen;				
		int m_Wait;

		int GetCheckSum(char* str );

		void MakeCmd(char* buf,int bufsize,char* station,char* pc,char* cmd,char* addr,short counts,char*msg);

		int Receive( char* buf, int bufsize, int waitlen );
		int Answer( char flg, char* station,char* pc );

		char m_buf[4096];

		unsigned char htoi( char *hexstr , short len );

};
#endif
