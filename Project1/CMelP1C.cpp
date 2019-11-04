
#include <stdio.h>
#include <conio.h>
#include "CMelP1C.h"

CMelsecP1C::CMelsecP1C(void)
{
	m_fOpen = FALSE;
}

CMelsecP1C::~CMelsecP1C()
{
	TRS232C::SetRTS(FALSE);
	TRS232C::SetDTR(FALSE);
	TRS232C::Close();
}

BOOL CMelsecP1C::Init(int portNo,int baudrate,int databit,int stopbit,int parity,int wait)
{
	// µ°ÌßÝ’†‚ÍOK
	if( m_fOpen ) return(TRUE);

	m_Wait = wait;

	PORTSETTING ps;
	memset( &ps, 0, sizeof(PORTSETTING));

	switch( baudrate ){
		case 1200:
			ps.BauRate=CBR_1200;				//1200bps
			break;
		case 2400:
			ps.BauRate=CBR_2400;				//1200bps
			break;
		case 4800:
			ps.BauRate=CBR_4800;				//1200bps
			break;
		case 19200:
			ps.BauRate=CBR_19200;				//1200bps
			break;
		case 38400:
			ps.BauRate=CBR_38400;				//1200bps
			break;
		case 9600:
		default:
			ps.BauRate=CBR_9600;				//1200bps
			break;
	}

	// DataBit
	switch( databit ){
		case 7:
			ps.DByte=7;						
			break;
		case 8:
		default:
			ps.DByte=8;						
			break;
	}

	// STOP BIT
	switch( stopbit){
		case 1:
			ps.StopBits=ONESTOPBIT;			
			break;
		case 2:
		default:
			ps.StopBits=TWOSTOPBITS;			
			break;
	}

	// PARITY
	switch( parity){
		case 1:
			ps.Parity=ODDPARITY;				
			break;
		case 2:									// 
			ps.Parity=EVENPARITY;				
			break;
		case 0:
		default:
			ps.Parity=NOPARITY;				
			break;
	}
	ps.fBinary = TRUE;
	ps.fOutxCtsFlow = TRUE;		// TRUE or FALSE
	ps.fOutxDsrFlow = TRUE;		// TRUE or FALSE
	ps.fDtrControl = DTR_CONTROL_HANDSHAKE;		// DTR_CONTROL_DISABLE or DTR_CONTROL_ENABLE or DTR_CONTROL_HANDSHAKE
	ps.fRtsControl = RTS_CONTROL_HANDSHAKE;		// RTS_CONTROL_ENABLE or RTS_CONTROL_DISABLEE or RTS_CONTROL_HANDSHAKE or RTS_CONTROL_TOGGLE


	if(!TRS232C::CommOpen((long)portNo,&ps)){
		return(FALSE);
	}

	TRS232C::SetRTS(TRUE);
	TRS232C::SetDTR(TRUE);

	TRS232C::ClearRcvBuffer();
	m_fOpen=TRUE;

	return(TRUE);
}

int CMelsecP1C::P4ReadW( char* buf,int bufsize,char* station,char* pcstation,char* addr, short counts)
{
	int ret,len;
	int retry=0,sum;
	char data[20],txt[10];

	if( m_fOpen == FALSE ) return(-1);
	if( counts <0 || counts > 64 ) return(-2);

	memset( data, 0, sizeof(data));
	while( retry < 8 ){
		MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "WR",addr,counts,"");

		TRS232C::ClearRcvBuffer();			
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );		
		if( ret ) return(-3);

		if( data[0] == STX ){
			len =0;
			memset( m_buf, 0, sizeof(m_buf) );
			while( data[0] != ETX ){
				ret = Receive( data, sizeof(data), 1 );		
				if( ret ) return(-4);							
				m_buf[len] = data[0];							
				if( ++len == sizeof(m_buf) ) return(-5); 						
			}

			ret = Receive( data, sizeof(data), 2 );
			if( ret ) return(-6);						

			sum = GetCheckSum( m_buf );
			wsprintf(txt, "%02X", sum);

			if( !memcmp( txt, data, 2 ) ){
				Answer(ACK,station,pcstation);
				break;
			}
			Answer(NAK,station,pcstation);
			retry++;									
			continue;
		}

		else if( data[0] == NAK ){
			ret = Receive( data, sizeof(data), 6 );		
			if( ret ) return(-7);
			retry++;									
			continue;
		}

		else return(-8);
	}

	memcpy( buf, m_buf+4,(int)(strlen(m_buf+4)-1) > bufsize ? bufsize:strlen(m_buf+4)-1);

	return(0);
}

int CMelsecP1C::P4WriteW( char* station, char* pcstation,char* addr, short counts, char* msg)
{
	int ret,len;
	int retry=0;
	char data[20];
	char initcom[5];

	initcom[0]=4;
	initcom[1]=0xD;
	initcom[2]=0xA;
	initcom[3]=0;

	if( m_fOpen == FALSE ) return(-1);
	if( counts <0 || counts > 64 ) return(-2);

	if( strlen(addr)+strlen(msg) + P4_HEADLEN >= sizeof(m_buf) ) return(-10);

	if( counts*4 != (int)strlen(msg) ) return(-11);

	MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "WW",addr,counts,msg);

	while( retry < 8 ){
		if( retry ){
			TRS232C::ClearRcvBuffer();				
			ret = TRS232C::Write( (BYTE*)initcom, 3 );
		}		
		TRS232C::ClearRcvBuffer();			
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );		
		if( ret ){
			if( retry++ < 8 )continue;
			return(-3);
		}
		if( data[0] == ACK )  len = 4 ;		
		else if( data[0] == NAK )  len = 6;
		else{
			if( retry++ < 8 )continue;
			return(-4);						
		}
		ret = Receive( data, sizeof(data), len );
		if( ret ){
			if( retry++ < 8 )continue;
			return(-5);
		}

		if( len == 6 ) retry++;

		else break;
	}
	return(0);
}

int CMelsecP1C::P4Send( char* station, char* pcstation,char* command,char* addr, short counts, char* msg)
{
	int ret,len;
	int retry=0;
	char data[20];
	char initcom[5];

	initcom[0]=4;
	initcom[1]=0xD;
	initcom[2]=0xA;
	initcom[3]=0;

	if( m_fOpen == FALSE ) return(-1);
	if( counts <0 ) return(-2);

	if( strlen(addr)+strlen(msg) + P4_HEADLEN+2 >= sizeof(m_buf) ) return(-10);


	MakeCmd( m_buf, sizeof(m_buf), station,pcstation, command,addr,counts,msg);

	while( retry < 8 ){
		if( retry ){
			TRS232C::ClearRcvBuffer();				
			ret = TRS232C::Write( (BYTE*)initcom, 3 );
		}		
		TRS232C::ClearRcvBuffer();			
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );		
		if( ret ){
			if( retry++ < 8 )continue;
			return(-3);
		}
		if( data[0] == ACK )  len = 6 ;		
		else if( data[0] == NAK )  len = 8;
		else{
			if( retry++ < 8 )continue;
			return(-4);					
		}
		ret = Receive( data, sizeof(data), len );
		if( ret ){
			if( retry++ < 8 )continue;
			return(-5);
		}
		if( len == 8 ) retry++;

		else break;
	}
	return(0);
}

int CMelsecP1C::P4ReadB( char* station,char* pcstation,char* addr)
{
	int ret,len;
	int retry=0,sum;
	char data[20],txt[10];
	char buf;

	if( m_fOpen == FALSE ) return(-1);

	memset( data, 0, sizeof(data));
	while( retry < 8 ){
		MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "BR",addr,8,"");

		TRS232C::ClearRcvBuffer();				
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );	
		if( ret ) return(-3);

		if( data[0] == STX ){
			len =0;
			memset( m_buf, 0, sizeof(m_buf) );
			while( data[0] != ETX ){
				ret = Receive( data, sizeof(data), 1 );		
				if( ret ) return(-4);							
				m_buf[len] = data[0];							
				if( ++len == sizeof(m_buf) ) return(-5); 						
			}

			ret = Receive( data, sizeof(data), 2 );
			if( ret ) return(-6);						

			sum = GetCheckSum( m_buf );
			wsprintf(txt, "%02X", sum);

			if( !memcmp( txt, data, 2 ) ){
				Answer(ACK,station,pcstation);
				break;
			}
			Answer(NAK,station,pcstation);
			retry++;									
			continue;
		}

		else if( data[0] == NAK ){
			ret = Receive( data, sizeof(data), 6 );		
			if( ret ) return(-7);
			retry++;									
			continue;
		}

		else return(-8);
	}

	buf=m_buf[4];

	return(buf-0x30);
}

int CMelsecP1C::P4WriteB( char* station, char* pcstation,char* addr, int onoff)
{
	int ret,len;
	int retry=0;
	char data[20];
	char send[10];

	if( m_fOpen == FALSE ) return(-1);

	if( !onoff ) strcpy(send,"0\0");
	else strcpy(send,"1\0");			

	MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "BW",addr,1,send);

	while( retry < 8 ){
		TRS232C::ClearRcvBuffer();				
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );		
		if( ret ) return(-3);

		if( data[0] == ACK )  len = 4 ;		
		else if( data[0] == NAK )  len = 6;
		else return(-4);					

		ret = Receive( data, sizeof(data), len );
		if( ret ) return(-5);

		if( len == 6 ) retry++;

		else break;
	}
	return(0);
}

int CMelsecP1C::Receive( char* buf, int bufsize, int waitlen )
{
	int theLen,prevLen=0;
	int counter = 0;

	if( m_fOpen == FALSE ) return(-1);
	if( waitlen <1 ) return(-2);
	if( bufsize < waitlen ) return(-2);

	memset( buf, 0, bufsize );

	while(1){
		theLen = TRS232C::ReadLenght();
		if( theLen >= waitlen ){
			break;
		}
		else{
			Sleep(15);
			if( theLen != prevLen ){	
				theLen = prevLen;
				counter=0;
			}
			else counter++;			
		}
		if( counter > 40 ) return(-3);
	}

	TRS232C::Read( (BYTE*)buf, waitlen );
	return(0);
}

int CMelsecP1C::Answer( char flg, char* station,char* pc )
{
	char buf[10];
	int ret;

	memset( buf, 0, sizeof(buf));

	buf[0]= flg;
	memcpy( buf+1, station, 2 );
	memcpy( buf+3, pc, 2 );

	ret = TRS232C::Write( (BYTE*)buf, strlen(buf) );
	return(0);
}

void CMelsecP1C::MakeCmd(char* buf,int bufsize,char* station,char* pc,char* cmd,char* addr,short counts,char*msg)
{
	char txt[10];
	int sum;

	memset( buf, 0, bufsize );
	buf[0] = ENQ;

	strcat( buf, station );
	strcat( buf, pc );

	strcat( buf, cmd );

	wsprintf( txt, "%d", m_Wait );
	memcpy( &buf[strlen(buf)], txt, 1 );

	strcat( buf, addr );

	wsprintf( txt, "%02X", counts );
	memcpy( &buf[strlen(buf)], txt, 2 );

	strcat( buf, msg );

	sum = GetCheckSum(&buf[1]);
	wsprintf( txt, "%02X", sum );
	memcpy( &buf[strlen(buf)], txt, 2 );
}

int CMelsecP1C::GetCheckSum(char* str )
{
	int data=0;
	int i,len;

	if( str==NULL ) return(0);

	len = strlen(str);
	if( len <=0 ) return(0);

	for( i=0; i< len; i++ )	data += (int)str[i];

	data = data & 0x000000ff;

	return(data);
}

int CMelsecP1C::Str2Mel(char* buf, unsigned int bufsize, char* str)
{
	unsigned int i,len;
	char txt[10];

	len = strlen(str);
	if( len*2 >= bufsize ) return(-1);

	memset( buf, 0, bufsize );

	for( i=0;i<len;i++){
		if( i+1==len )
			wsprintf( txt,"00");
		else
			wsprintf( txt,"%02X",(unsigned char)str[i+1]);	
		memcpy( &buf[i*2], txt, 2 );

		wsprintf( txt,"%02X",(unsigned char)str[i]);		
		i++;
		memcpy( &buf[i*2], txt, 2 );
	}

	return(0);

}

int CMelsecP1C::Mel2Str(char* buf, unsigned int bufsize, char* melstr)
{
	unsigned int i,len;

	len = strlen(melstr);
	if( bufsize<=len/2 ) return(-1);

	memset( buf, 0, bufsize );

	for( i=0;i<len;i+=2){
		if( i+2>=len )
			buf[strlen(buf)]=0x20;
		else
			buf[strlen(buf)]=htoi( &melstr[i+2],2);

		buf[strlen(buf)]=htoi( &melstr[i],2);
		i+=2;
	}
	return(0);
}

unsigned char CMelsecP1C::htoi( char *hexstr , short len )
{
	unsigned char ret = 0;
	short i;

	if( len > 4 ) return 0;
	for( i = 0 ; i < len ; i++ ){
		ret *= 16;
		if( hexstr[i] < 'A' ) ret += hexstr[i] - '0';
		else
		if( hexstr[i] < 'a' ) ret += hexstr[i] - 'A'+10;
		else
			ret += hexstr[i] - 'a'+10;
	}
	return(ret);
}


BOOL CMelsecP1C::ULChg( char* buf )
{
	char tmp[8];

	if( strlen(buf)<8 )
		return FALSE;

	memcpy( tmp , buf , 8);
	memcpy( buf+0 , tmp+4 , 4 );
	memcpy( buf+4 , tmp+0 , 4 );
	
	return TRUE;
}
