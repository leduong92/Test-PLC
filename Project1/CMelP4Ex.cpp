
#include <stdio.h>
#include <conio.h>
#include "CMelP4Ex.h"

CMelsecP4Ex::CMelsecP4Ex(void)
{
	m_fOpen=FALSE;
}

CMelsecP4Ex::~CMelsecP4Ex()
{
	TRS232C::SetRTS(FALSE);
	TRS232C::SetDTR(FALSE);
	TRS232C::Close();
}


void CMelsecP4Ex::ComClose(void)
{
	if( m_fOpen ){
		TRS232C::SetRTS(FALSE);
		TRS232C::SetDTR(FALSE);
		TRS232C::Close();
		m_fOpen=FALSE;
	}
}

BOOL CMelsecP4Ex::Init(int portNo,int baudrate,int databit,int stopbit,int parity,int wait,BOOL ctrl_line/*=TRUE*/)
{
	if( m_fOpen ) return(TRUE);

	m_Wait = wait;

	PORTSETTING ps;
	memset( &ps, 0, sizeof(PORTSETTING));

	switch( baudrate ){
		case 300:
			ps.BauRate=CBR_300;				//1200bps
			break;
		case 600:
			ps.BauRate=CBR_600;				//1200bps
			break;
		case 1200:
			ps.BauRate=CBR_1200;				//1200bps
			break;
		case 2400:
			ps.BauRate=CBR_2400;				//bps
			break;
		case 4800:
			ps.BauRate=CBR_4800;				//bps
			break;
		case 14400:
			ps.BauRate=CBR_14400;				//bps
			break;
		case 19200:
			ps.BauRate=CBR_19200;				//bps
			break;
		case 28800:
			ps.BauRate=28800;				//bps
			break;
		case 38400:
			ps.BauRate=CBR_38400;				//bps
			break;
		case 57600:
			ps.BauRate=CBR_57600;				//bps
			break;
		case 115200:
			ps.BauRate=CBR_115200;				//bps
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
		case 2:									
			ps.Parity=EVENPARITY;				
			break;
		case 0:
		default:
			ps.Parity=NOPARITY;				
			break;
	}
	ps.fBinary = TRUE;

	if( ctrl_line == TRUE){
		ps.fOutxCtsFlow = TRUE;		// TRUE or FALSE
		ps.fOutxDsrFlow = TRUE;		// TRUE or FALSE
		ps.fDtrControl = DTR_CONTROL_HANDSHAKE;		// DTR_CONTROL_DISABLE or DTR_CONTROL_ENABLE or DTR_CONTROL_HANDSHAKE
		ps.fRtsControl = RTS_CONTROL_HANDSHAKE;		// RTS_CONTROL_ENABLE or RTS_CONTROL_DISABLEE or RTS_CONTROL_HANDSHAKE or RTS_CONTROL_TOGGLE
	}
	else{
		ps.fOutxCtsFlow = FALSE;		// TRUE or FALSE
		ps.fOutxDsrFlow = FALSE;		// TRUE or FALSE
		ps.fDtrControl = DTR_CONTROL_DISABLE;		// DTR_CONTROL_DISABLE or DTR_CONTROL_ENABLE or DTR_CONTROL_HANDSHAKE
		ps.fRtsControl = RTS_CONTROL_DISABLE;		// RTS_CONTROL_ENABLE or RTS_CONTROL_DISABLEE or RTS_CONTROL_HANDSHAKE or RTS_CONTROL_TOGGLE
	}

	if(!TRS232C::CommOpen((long)portNo,&ps)){
		return(FALSE);
	}

	if( ctrl_line == TRUE){
		TRS232C::SetRTS(TRUE);
		TRS232C::SetDTR(TRUE);
	}
	TRS232C::ClearRcvBuffer();
	m_fOpen=TRUE;

	return(TRUE);
}

int CMelsecP4Ex::P4ReadW( char* buf,int bufsize,char* station,char* pcstation,char* addr, short counts)
{
	int ret,len;
	int retry=0,sum;
	char data[20],txt[10];
	char initcom[5];

	initcom[0]=4;
	initcom[1]=0xD;
	initcom[2]=0xA;
	initcom[3]=0;

	if( m_fOpen == FALSE ) return(-1);
	if( counts <0 || counts > 64 ) return(-2);

	memset( data, 0, sizeof(data));
	while( retry < 8 ){
		if( retry ){
			TRS232C::ClearRcvBuffer();			
			ret = TRS232C::Write( (BYTE*)initcom, 3 );
		}		
		if(strlen(addr) == 5 )
			MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "WR",addr,counts,"");
		else MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "QR",addr,counts,"");
		TRS232C::ClearRcvBuffer();			
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );		
		if( ret ) return(-3);

		if( data[0] == STX ){
			len =0;
			memset( m_buf, 0, sizeof(m_buf) );
			while( data[0] != ETX ){
				ret = Receive( data, sizeof(data), 1 );			
				if( ret ){
					return(-4);						
				}
				m_buf[len] = data[0];							
				if( ++len == sizeof(m_buf) ) return(-5); 						
			}

			ret = Receive( data, sizeof(data), 4 );
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
			ret = Receive( data, sizeof(data), 8 );			
			if( ret ) return(-7);
			retry++;										
			continue;
		}

		else return(-8);
	}
	if(retry >= 8) return(-9);

	memcpy( buf, m_buf+4,(int)(strlen(m_buf+4)-1) > bufsize ? bufsize:strlen(m_buf+4)-1);

	return(0);
}

int CMelsecP4Ex::P4WriteW( char* station, char* pcstation,char* addr, short counts, char* msg)
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

	if(strlen(addr) == 5 )
		MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "WW",addr,counts,msg);
	else
		MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "QW",addr,counts,msg);

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
	if(retry >= 8) return(-9);
	return(0);
}

int CMelsecP4Ex::P4Send( char* station, char* pcstation,char* command,char* addr, short counts, char* msg)
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
	if(retry >= 8) return(-9);
	return(0);
}


int CMelsecP4Ex::P2C4ReadW( char* buf,int bufsize,char* station,char* pcstation,char* addr, short counts)
{
	int ret,len;
	int retry=0;
	char data[4096];
	char lsum[4096];
	char initcom[5];
	char st;
	char sumt[16];

	initcom[0]=4;
	initcom[1]=0xD;
	initcom[2]=0xA;
	initcom[3]=0;

	if( m_fOpen == FALSE ) return(-1);
	if( counts <0 || counts > 960 ) return(-2);

	MakeCmd2C( m_buf, sizeof(m_buf), station,pcstation, "2",addr,counts,"");

	while( retry <= 8 ){
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
		st=data[0];
		if( data[0] == STX )len = counts*4+9;	
		else if( data[0] == NAK )  len = 12;
		else{
			if( retry++ < 8 )continue;
			return(-4);						
		}
		ret = Receive( data, sizeof(data), len );
		if( ret ){
			if( retry++ < 8 )continue;
			return(-5);
		}
		if( st==STX ){
			memset(lsum,0,sizeof(lsum));
			memcpy(lsum,data,len-2);
			sprintf(sumt,"%02X",GetCheckSum(lsum));
		}

		if( st == NAK || memcmp(sumt,data+len-2,2) ) retry++;

		else break;
	}
	memcpy( buf, lsum+6,(int)(strlen(lsum+6)-1) > bufsize ? bufsize:strlen(lsum+6)-1);

	if(retry >= 8) return(-9);
	return(0);
}

int CMelsecP4Ex::P2C4WriteW( char* station, char* pcstation,char* addr, short counts, char* msg)
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
	if( counts <0 || counts > 960 ) return(-2);

	if( strlen(addr)+strlen(msg) + P4_HEADLEN >= sizeof(m_buf) ) return(-10);

	if( counts*4 != (int)strlen(msg) ) return(-11);

	MakeCmd2C( m_buf, sizeof(m_buf), station,pcstation, "4",addr,counts,msg);

	while( retry <= 8 ){
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
		else if( data[0] == NAK )  len = 12;	
		else{
			if( retry++ < 8 )continue;
			return(-4);						
		}
		ret = Receive( data, sizeof(data), len );
		if( ret ){
			if( retry++ < 8 )continue;
			return(-5);
		}
		if( len == 12 ) retry++;

		else break;
	}
	if(retry >= 8) return(-9);
	return(0);
}

int CMelsecP4Ex::P4ReadB( char* station,char* pcstation,char* addr)
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

			ret = Receive( data, sizeof(data), 4 );
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
			ret = Receive( data, sizeof(data), 8 );			
			if( ret ) return(-7);
			retry++;										
			continue;
		}

		else return(-8);
	}

	if(retry >= 8) return(-9);

	buf=m_buf[4];

	return(buf-0x30);
}

int CMelsecP4Ex::P4ReadBB(  char* buf,int bufsize,char* station,char* pcstation,char* addr,short cnt)
{
	int ret,len;
	int retry=0,sum;
	char data[20],txt[10];

	if( m_fOpen == FALSE ) return(-1);

	memset( data, 0, sizeof(data));
	while( retry < 8 ){
		MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "BR",addr,cnt,"");

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

			ret = Receive( data, sizeof(data), 4 );
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
			ret = Receive( data, sizeof(data), 8 );			
			if( ret ) return(-7);
			retry++;									
			continue;
		}

		else return(-8);
	}

	if(retry >= 8) return(-9);

	memcpy( buf, m_buf+4,(int)(strlen(m_buf+4)-1) > bufsize ? bufsize:strlen(m_buf+4)-1);

	return(0);
}

int CMelsecP4Ex::P4WriteB( char* station, char* pcstation,char* addr, int onoff)
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

		if( data[0] == ACK )  len = 6 ;		
		else if( data[0] == NAK )  len = 8;
		else return(-4);						

		ret = Receive( data, sizeof(data), len );
		if( ret ) return(-5);

		if( len == 8 ) retry++;

		else break;
	}
	if(retry >= 8) return(-9);
	return(0);
}

int CMelsecP4Ex::P4WriteBB( char* station, char* pcstation,char* addr, int cnt,char* dat)
{
	int ret,len;
	int retry=0;
	char data[20];

	if( m_fOpen == FALSE ) return(-1);

	if( (int)strlen(dat) != cnt ) return(-2);

	MakeCmd( m_buf, sizeof(m_buf), station,pcstation, "BW",addr,cnt,dat);

	while( retry < 8 ){
		TRS232C::ClearRcvBuffer();			
		ret = TRS232C::Write( (BYTE*)m_buf, strlen(m_buf) );

		ret = Receive( data, sizeof(data), 1 );		
		if( ret ) return(-3);

		if( data[0] == ACK )  len = 6 ;		
		else if( data[0] == NAK )  len = 8;	
		else return(-4);						

		ret = Receive( data, sizeof(data), len );
		if( ret ) return(-5);

		if( len == 8 ) retry++;

		else break;
	}
	if(retry >= 8) return(-9);
	return(0);
}

int CMelsecP4Ex::Receive( char* buf, int bufsize, int waitlen )
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

int CMelsecP4Ex::Answer( char flg, char* station,char* pc )
{
	char buf[10];
	int ret;

	memset( buf, 0, sizeof(buf));

	buf[0]= flg;
	memcpy( buf+1, station, 2 );
	memcpy( buf+3, pc, 2 );
	buf[5]=CR;
	buf[6]=LF;

	ret = TRS232C::Write( (BYTE*)buf, strlen(buf) );
	return(0);
}

void CMelsecP4Ex::MakeCmd(char* buf,int bufsize,char* station,char* pc,char* cmd,char* addr,short counts,char*msg)
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

	buf[strlen (buf)] = CR;
	buf[strlen (buf)] = LF;
}

void CMelsecP4Ex::MakeCmd2C(char* buf,int bufsize,char* station,char* pc,char* cmd,char* addr,short counts,char*msg)
{
	char txt[10];
	int sum;

	memset( buf, 0, bufsize );
	buf[0] = ENQ;
	buf[1] = 'F';
	buf[2] = 'B';

	strcat( buf, station );
	strcat( buf, pc );

	strcat( buf, cmd );
	strcat( buf, addr );

	wsprintf( txt, "%04X", counts );
	memcpy( &buf[strlen(buf)], txt, 4 );

	strcat( buf, msg );

	sum = GetCheckSum(&buf[1]);
	wsprintf( txt, "%02X", sum );
	memcpy( &buf[strlen(buf)], txt, 2 );

	buf[strlen (buf)] = CR;
	buf[strlen (buf)] = LF;
}

int CMelsecP4Ex::GetCheckSum(char* str )
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

int CMelsecP4Ex::Str2Mel(char* buf, unsigned int bufsize, char* str)
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

int CMelsecP4Ex::Mel2Str(char* buf, unsigned int bufsize, char* melstr)
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


unsigned char CMelsecP4Ex::htoi( char *hexstr , short len )
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

int CMelsecP4Ex::UStr2Mel(char* buf, unsigned int bufsize, unsigned char* str)
{
	unsigned int i,len;
	char txt[10];

	len = strlen((char*)str);
	if( len*2 >= bufsize ) return(-1);

	memset( buf, 0, bufsize );

	for( i=0;i<len;i++){
		// ã‰º‚ð“ü‚ê‘Ö‚¦‚é
		if( i+1==len )
			wsprintf( txt,"00");
		else
			wsprintf( txt,"%02X",(unsigned)str[i+1] & 0xFF);
		memcpy( &buf[i*2], txt, 2 );

		wsprintf( txt,"%02X",(unsigned)str[i] & 0xFF);
		i++;
		memcpy( &buf[i*2], txt, 2 );
	}

	return(0);

}


BOOL CMelsecP4Ex::ULChg( char* buf )
{
	char tmp[8];

	if( strlen(buf)<8 )
		return FALSE;

	memcpy( tmp , buf , 8);
	memcpy( buf+0 , tmp+4 , 4 );
	memcpy( buf+4 , tmp+0 , 4 );
	
	return TRUE;
}
